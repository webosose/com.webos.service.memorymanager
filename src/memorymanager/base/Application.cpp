// Copyright (c) 2018-2020 LG Electronics, Inc.
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

#include "Application.h"

#include "util/Logger.h"

bool Application::isCloseable(const string& status)
{
    if (status == "stop" || status == "close" || status == "splash" || status == "launch")
        return false;

    return true;
}

bool Application::isCloseable() 
{ 
    return isCloseable(m_status); 
}

int Application::getStatusPriority(const string& status)
{
    if (!isCloseable(status))
        return 0;
    else if (status == "pause")
        return 1;
    else if (status == "foreground")
        return 2;
    else if (status == "background")
        return 3;
    else if (status == "preload")
        return 4;
    else
        return 5;
}

int Application::getTypePriority(const string& type)
{
    if (type == "web")
        return 0;
    else if (type.find("native") != std::string::npos)
        return 1;
    else
        return 2;
}

bool Application::compare(const Application& a, const Application& b)
{
    if (a.m_status == b.m_status) {
        if (a.m_type == b.m_type)
            return a.m_time > b.m_time;
        else
            return getTypePriority(a.m_type) < getTypePriority(b.m_type);
    } else {
        return getStatusPriority(a.m_status) < getStatusPriority(b.m_status);
    }
}

void Application::setProperty(string instanceId, string appId, string type, int pid, int displayId)
{
    m_instanceId = instanceId;
    m_appId = appId;
    m_type = type;
    m_displayId = displayId;
    m_pid = pid;
}

void Application::setStatus(const string& status)
{
    if (status == "foreground") {
        m_time = Time::getSystemTime();
    }

    m_status = status;
}

void Application::print()
{
    char buf[1024];
    snprintf(buf, 1024, "%6.6s %-30.30s | %10.10s %10s %5d %5d",
            m_instanceId.c_str(), m_appId.c_str(),
            m_status.c_str(), m_type.c_str(),
            m_pid, m_time);

    Logger::verbose(buf, "Application");
}

void Application::print(JValue& json)
{
    json.put("instanceId", m_instanceId);
    json.put("appId", m_appId);
    json.put("status", m_status);
    json.put("type", m_type);
    json.put("pid", m_pid);
    json.put("time", m_time);
    json.put("displayId", m_displayId);
}

Application::Application()
    : m_instanceId(""),
      m_appId(""),
      m_type(""),
      m_pid(0),
      m_displayId(0),
      m_status(""),
      m_time(0)
{
    setClassName("Application");
}

Application::~Application()
{

}
