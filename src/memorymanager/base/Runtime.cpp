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

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "MemoryManager.h"
#include "base/Runtime.h"
#include "sam/SAM.h"

#include "util/Proc.h"
#include "util/Logger.h"
#include "util/Cgroup.h"

void BaseProcess::setPid(const int pid)
{
    m_pids.clear();
    m_pids.push_back(pid);
    m_pids.sort();
}

const string BaseProcess::toString(const list<int>& pids)
{
    string ret;
    for (int pid: pids)
        ret += to_string(pid) + " ";
    boost::trim(ret);
    return ret;
}

void BaseProcess::updateMemStat()
{
    map<string, string> smaps;
    m_pssKb = 0;

    for (auto& pid : m_pids) {
        Proc::getSmapsRollup(pid, smaps);

        auto it_pss = smaps.find("Pss");
        if (it_pss == smaps.end())
            m_pssKb += 0;
        else
            m_pssKb += stoul(it_pss->second);
    }
}

BaseProcess::BaseProcess(const list<int>& pids)
    :m_pids(pids),
     m_pssKb(0)
{
    m_pids.sort();
}

void Application::print()
{
    char buf[1024];
    snprintf(buf, 1024, "%6.6s %-30.30s: %10lu kb: %s %10.10s %10s",
             m_instanceId.c_str(), m_appId.c_str(),
             m_pssKb,
             BaseProcess::toString(m_pids).c_str(),
             m_status.c_str(), m_type.c_str());

    Logger::verbose(buf, getClassName());
}

void Application::print(JValue& json)
{
    json.put("instanceId", m_instanceId);
    json.put("appId", m_appId);
    json.put("status", m_status);
    json.put("type", m_type);
    json.put("pid", BaseProcess::toString(m_pids).c_str());
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
    if (m_appId == compare.getAppId() && m_pids == compare.m_pids)
        return true;
    else
        return false;
}

Application::Application(const string& instanceId, const string& appId,
                         const string& type, const string& status,
                         const int pid)
    :BaseProcess({pid}),
     m_instanceId(instanceId),
     m_appId(appId),
     m_type(type),
     m_status(status)
{
    setClassName("Application");
}

void Service::print()
{
    char buf[1024];
    snprintf(buf, 1024, "%41.41s: %10lu kb: %s",
             m_serviceId.c_str(),
             m_pssKb,
             BaseProcess::toString(m_pids).c_str());

    Logger::verbose(buf, getClassName());
}

void Service::print(JValue& json)
{
    json.put("serviceId", m_serviceId);
    json.put("pid", BaseProcess::toString(m_pids).c_str());
}

const string& Service::getServiceId()
{
    return m_serviceId;
}

Service::Service(const string& serviceId, const list<int>& pids)
    :BaseProcess(pids),
     m_serviceId(serviceId)
{
    setClassName("Service");
}

void Runtime::updateMemStat()
{
    for (auto it = m_services.begin(); it != m_services.end(); it++)
        (*it)->updateMemStat();

    for (auto it = m_applications.begin(); it != m_applications.end(); it++)
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
                                RUNTIME_CHANGE_APP_CLOSE);
        return true;
    }

    return false;
}

void Runtime::addService(Service* service)
{
    m_services.push_back(service);
}

int Runtime::countService()
{
    return m_services.size();
}

void Runtime::printService(JValue& json)
{
}

void Runtime::printService()
{
    for (auto it = m_services.begin(); it != m_services.end(); it++)
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
                            RUNTIME_CHANGE_APP_ADD);
}

bool Runtime::updateApp(const string& appId, const string& event)
{
    auto it = m_applications.begin();
    for (; it != m_applications.end(); ++it) {
        if (it->getAppId() == appId)
            break;
    }

    if (it == m_applications.end())
        return false;

    enum RuntimeChange change;

    if (event == "stop") {
        m_applications.remove(*it);
        change = RUNTIME_CHANGE_APP_REMOVE;
    } else {
        if (event == "foreground") {
            m_applications.splice(m_applications.end(), m_applications, it);
        } else {
            list<Application>::reverse_iterator rit = findFirstForeground();
            m_applications.splice(--rit.base(), m_applications, it);
        }

        it->setStatus(event);
        change = RUNTIME_CHANGE_APP_UPDATE;
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

int Runtime::countApp()
{
    return m_applications.size();
}

void Runtime::printApp(JValue& json)
{
    for (auto it = m_applications.begin(); it != m_applications.end(); it++) {
        JValue obj = pbnjson::Object();
        it->print(obj);
        json.append(obj);
    }
}

void Runtime::printApp()
{
    for (auto it = m_applications.begin(); it != m_applications.end(); it++)
        it->print();
}

Runtime::Runtime(Session &session):m_session(session)
{
    setClassName("Runtime");

    /* Create service list when session runtime is created once */
    const string p = session.getPath();

    /* TODO busy wating */
    while (!boost::filesystem::exists(p))
        Logger::normal(p + " not exist, busy wating...", getClassName());

    map<string, list<int>> comm_pids;
    Cgroup::iterateDir(comm_pids, p);

    for (auto& [comm, pids] : comm_pids) {
        Service* svc = new Service(comm, pids);
        addService(svc);
    }
    updateMemStat(); //TODO: move to sysInfo
}

Runtime::~Runtime()
{
    for (auto &svc:m_services)
        delete svc;
    m_services.clear();
}
