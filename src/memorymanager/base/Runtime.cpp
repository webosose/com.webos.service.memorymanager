// Copyright (c) 2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <chrono>
#include <thread>

#include "MemoryManager.h"
#include "base/Runtime.h"
#include "sam/SAM.h"

#include "util/Proc.h"
#include "util/Logger.h"
#include "util/Cgroup.h"
#include "util/LinuxProcess.h"

const string Runtime::WAM_SERVICE_ID = "webapp-mgr.service";
const string Runtime::SAM_SERVICE_ID = "sam.service";

long BaseProcess::getPssValue(const int pid)
{
    map<string, string> smaps;

    if (!Proc::getSmapsRollup(pid, smaps))
        return -1;

    auto it_pss = smaps.find("Pss");
    if (it_pss == smaps.end())
        return 0;
    else
        return stoul(it_pss->second);
}

void Application::updateMemStat()
{
    long ret = getPssValue(m_pid);

    if (ret > 0 || ret == 0)
        m_pssKb = static_cast<unsigned long>(ret);
    else
        m_pssKb = 0;
}

void Application::print()
{
    char buf[1024];
    snprintf(buf, 1024, "%6.6s %-30.30s: %10lu kb: %5d %10.10s %10s",
             m_instanceId.c_str(), m_appId.c_str(),
             m_pssKb, m_pid,
             m_status.c_str(), m_type.c_str());

    Logger::verbose(buf, getClassName());
}

void Application::print(JValue& json)
{
    json.put("instanceId", m_instanceId);
    json.put("appId", m_appId);
    json.put("status", m_status);
    json.put("type", m_type);
    json.put("pid", m_pid);
    json.put("pss", to_string(m_pssKb));
}

void Application::setPid(const int pid)
{
    m_pid = pid;
}

void Application::setStatus(const string& status)
{
    m_status = status;
}

void Application::setType(const string& type)
{
    m_type = type;
}

bool Application::operator==(const Application& compare)
{
    if (m_appId == compare.getAppId() && m_instanceId == compare.getInstanceId())
        return true;
    else
        return false;
}

Application::Application(const string& instanceId, const string& appId,
                         const string& type, const string& status,
                         const int pid)
    :m_instanceId(instanceId),
     m_appId(appId),
     m_type(type),
     m_status(status),
     m_pid(pid)
{
    setClassName("Application");
    m_pssKb = 0;
}

template<typename T, typename U>
void Service::toString(map<T, U>& pMap, string& str1, string& str2)
{
    string ret1 = "", ret2 = "";

    auto it = pMap.cbegin();
    for (; it != pMap.cend(); ++it) {
        ret1 += to_string(it->first) + " ";
        ret2 += to_string(it->second) + " ";
    }

    boost::trim(ret1);
    boost::trim(ret2);

    str1 = std::move(ret1);
    str2 = std::move(ret2);
}

void Service::updateMemStat()
{
    auto it = m_pidPss.begin();
    while (it != m_pidPss.end()) {
        long localPss = getPssValue(it->first);

        if (localPss < 0) {
            it = m_pidPss.erase(it);
            continue;
        } else {
            it->second = (unsigned long)localPss;
            ++it;
        }
    }
}

void Service::print()
{
    char buf[1024];
    unsigned long pssSum = 0;
    string pss = "", pid = "";

    for (auto it = m_pidPss.cbegin(); it != m_pidPss.cend(); ++it)
        pssSum += it->second;

    toString(m_pidPss, pid, pss);
    snprintf(buf, 1024, "%41.41s: %10lu kb: %s",
             m_serviceId.c_str(), pssSum, pid.c_str());

    Logger::verbose(buf, getClassName());
}

void Service::print(JValue& json)
{
    string pss = "", pid = "";
    toString(m_pidPss, pid, pss);

    json.put("serviceId", m_serviceId);
    json.put("pid", pid.c_str());
    json.put("pss", pss.c_str());
}

Service::Service(const string& serviceId, const list<int>& pids)
    : m_serviceId(serviceId)
{
    setClassName("Service");

    for (int pid : pids)
        m_pidPss.insert(make_pair(pid, 0));
}

void Runtime::updateMemStat()
{
    /* Update Service Memory Stat */
    auto it = m_services.begin();
    while (it != m_services.end()) {
        (*it)->updateMemStat();

        /* If pid is fully empty, remove Service object */
        if ((*it)->getPidPss().empty()) {
            delete (*it);
            it = m_services.erase(it);
        } else {
            ++it;
        }
    }

    /* Update Application Memory Stat */
    for (auto it = m_applications.begin(); it != m_applications.end(); ++it)
        it->updateMemStat();
}

bool Runtime::reclaimMemory(bool critical)
{
    if (m_applications.empty())
        return false;

    auto it = m_applications.front();

    if (it.getStatus() == "foreground" && !critical)
        return false;

    if (m_session.m_sam->close(it.getAppId(), it.getInstanceId())) {
        MemoryManager* mm = MemoryManager::getInstance();
        mm->handleRuntimeChange(it.getAppId(), it.getInstanceId(),
                                RuntimeChange::APP_CLOSE);
        return true;
    }

    return false;
}

void Runtime::clearReservedPid(void)
{
    m_reservedPids.clear();
}

void Runtime::addReservedPid(const int pid)
{
    m_reservedPids.push_back(pid);
}

void Runtime::addService(Service* service)
{
    m_services.push_back(service);
}

void Runtime::updateService(const string& serviceId, const int pid)
{
    auto it = m_services.begin();
    for (; it != m_services.end(); ++it) {
        if ((*it)->getServiceId() == serviceId)
            break;
    }

    if (it == m_services.end())
        return;

    /* Erase matching pid from map (m_pidPss) */
    ((*it)->getPidPss()).erase(pid);
}

void Runtime::createService(Session *session)
{
    const string p = session->getPath();
    map<string, list<int>> comm_pids;

    Cgroup::iterateDir(comm_pids, p);

    /* Destroy previous service list */
    for (auto &svc:m_services)
        delete svc;
    m_services.clear();

    /* Create service list again */
    for (const auto& mcp : comm_pids) {
        string comm = mcp.first;
        list<int> pids = mcp.second;
        Service* svc = new Service(comm, pids);
        addService(svc);
    }

    /*
     * Forked proecss from WAM or SAM may exist in both Service and
     * Application list. Remove duplicated pid from Service list.
     */
    for (auto it = m_reservedPids.begin(); it != m_reservedPids.end(); ++it) {
        updateService(SAM_SERVICE_ID, *it);
        updateService(WAM_SERVICE_ID, *it);
    }
}

int Runtime::countService()
{
    return m_services.size();
}

void Runtime::printService(JValue& json)
{
    for (auto it = m_services.cbegin(); it != m_services.cend(); ++it) {
        JValue obj = pbnjson::Object();
        (*it)->print(obj);
        json.append(obj);
    }
}

void Runtime::printService()
{
    for (auto it = m_services.cbegin(); it != m_services.cend(); ++it)
        (*it)->print();
}

void Runtime::addApp(Application& app)
{
    list<Application>::reverse_iterator insertPos = findFirstForeground();

    if (app.getStatus() == "foreground") {
        m_applications.push_back(app);
    } else {
        if (insertPos != m_applications.rend()) {
            if (m_applications.size() == 1) {
                m_applications.push_front(app);
            } else {
                m_applications.insert(--insertPos.base(), app);
            }
        } else {
            m_applications.push_back(app);
        }
    }

    MemoryManager* mm = MemoryManager::getInstance();
    mm->handleRuntimeChange(app.getAppId(), app.getInstanceId(),
                            RuntimeChange::APP_ADD);
}

bool Runtime::updateApp(const string& appId, const string& instanceId,
                        const string& event)
{
    auto it = m_applications.begin();
    for (; it != m_applications.end(); ++it) {
        if (it->getAppId() == appId && it->getInstanceId() == instanceId)
            break;
    }

    if (it == m_applications.end())
        return false;

    enum RuntimeChange change;

    if (event == "stop") {
        m_applications.remove(*it);
        change = RuntimeChange::APP_REMOVE;
    } else {
        if (event == "foreground") {
            m_applications.splice(m_applications.end(), m_applications, it);
        } else {
            list<Application>::reverse_iterator rit = findFirstForeground();
            m_applications.splice(--rit.base(), m_applications, it);
        }

        it->setStatus(event);
        change = RuntimeChange::APP_UPDATE;
    }

    MemoryManager* mm = MemoryManager::getInstance();
    mm->handleRuntimeChange(it->getAppId(), it->getInstanceId(), change);
    return true;
}

list<Application>::reverse_iterator Runtime::findFirstForeground()
{
    list<Application>::reverse_iterator rit = m_applications.rbegin();
    for (; rit != m_applications.rend(); ++rit) {
        if (rit->getStatus() == "foreground") {
            break;
        }
    }

    return rit;
}

const string Runtime::findFirstForegroundAppId()
{
    list<Application>::reverse_iterator rit = m_applications.rbegin();
    for (; rit != m_applications.rend(); ++rit) {
        if (rit->getStatus() == "foreground") {
            return rit->getAppId();
        }
    }

    return "";
}

int Runtime::countApp()
{
    return m_applications.size();
}

void Runtime::printApp(JValue& json)
{
    for (auto it = m_applications.begin(); it != m_applications.end(); ++it) {
        JValue obj = pbnjson::Object();
        it->print(obj);
        json.append(obj);
    }
}

void Runtime::printApp()
{
    for (auto it = m_applications.begin(); it != m_applications.end(); ++it)
        it->print();
}

void Runtime::setAppDefaultStatus(const string& foregroundAppId)
{
    for (auto it = m_applications.begin(); it != m_applications.end(); ++it) {
        if (it->getAppId() == foregroundAppId)
            it->setStatus("foreground");
        else
            it->setStatus("background");
    }
}

Runtime::Runtime(Session &session):m_session(session)
{
    setClassName("Runtime");
}

Runtime::~Runtime()
{
}
