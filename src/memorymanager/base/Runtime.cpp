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

#include "MemoryManager.h"
#include "base/Runtime.h"
#include "sam/SAM.h"

#include "util/Proc.h"
#include "util/Logger.h"

void BaseProcess::setPid(const int pid)
{
    m_pid = pid;
}

void BaseProcess::updateMemStat()
{
    map<string, string> smaps;

    Proc::getSmapsRollup(m_pid, smaps);

    auto it_pss = smaps.find("Pss");
    if (it_pss == smaps.end())
        m_pssKb = 0;
    else
        m_pssKb = stoul(it_pss->second);
}

BaseProcess::BaseProcess(int pid)
    :m_pid(pid)
{

}

void Application::print()
{
    char buf[1024];
    snprintf(buf, 1024, "%6.6s %-30.30s %10.10s %10s %5d",
            m_instanceId.c_str(), m_appId.c_str(),
            m_status.c_str(), m_type.c_str(),
            m_pid);

    Logger::verbose(buf, getClassName());
}

void Application::print(JValue& json)
{
    json.put("instanceId", m_instanceId);
    json.put("appId", m_appId);
    json.put("status", m_status);
    json.put("type", m_type);
    json.put("pid", m_pid);
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
    if (m_appId == compare.getAppId() && m_pid == compare.m_pid)
        return true;
    else
        return false;
}

Application::Application(string instanceId, string appId, string type, string status, int pid)
    :BaseProcess(pid),
     m_instanceId(instanceId),
     m_appId(appId),
     m_type(type),
     m_status(status)
{
    setClassName("Application");
}

Application::Application(string instanceId, string appId, string status)
    :BaseProcess(0),
     m_instanceId(instanceId),
     m_appId(appId),
     m_status(status)
{
    setClassName("Application");
}

void Service::print()
{
    char buf[1024];
    snprintf(buf, 1024, "%30.30s %5d",
            m_serviceId.c_str(),
            m_pid);

    Logger::verbose(buf, getClassName());
}

void Service::print(JValue& json)
{
    json.put("serviceId", m_serviceId);
    json.put("pid", m_pid);
}

const string& Service::getServiceId()
{
    return m_serviceId;
}

Service::Service(string serviceId, int pid)
    :BaseProcess(pid),
     m_serviceId(serviceId)
{
    setClassName("Service");
}

void Runtime::updateMemStat()
{
    for (auto it = m_services.begin(); it != m_services.end(); it++)
        it->updateMemStat();

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

void Runtime::addService(Service& service)
{
    m_services.push_back(service);
}

int Runtime::countService()
{
    return m_services.size();
}

void Runtime::printService(JValue& json)
{
    /* TODO */
}

void Runtime::printService()
{
    for (auto it = m_services.begin(); it != m_services.end(); it++)
        it->print();
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
}
