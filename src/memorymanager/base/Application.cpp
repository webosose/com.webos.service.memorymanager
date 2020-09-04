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

Application::Application()
    : m_appId("unknown"),
      m_type("unknown"),
      m_status("unknown"),
      m_displayId(0),
      m_pid(0),
      m_time(0),
      m_context(0)
{
    setClassName("Application");
}

Application::Application(string appId)
    : Application()
{
    m_appId = appId;
}

Application::~Application()
{
}

void Application::print()
{
    static char buffer[1024];
    int result;
    if (m_sessionId.empty())
        result = sprintf(buffer , "%-30s %15s %10s %10d %10d", m_appId.c_str(), m_status.c_str(), m_type.c_str(), m_pid, m_time);
    else
        result = sprintf(buffer , "%-40s %-30s %15s %10s %10d %10d", m_sessionId.c_str(), m_appId.c_str(), m_status.c_str(), m_type.c_str(), m_pid, m_time);
    if (result > 0) {
        Logger::verbose(buffer, getClassName());
    } else {
        Logger::warning("Failed in sprintf", getClassName());
    }

}

void Application::print(JValue& json)
{
    if (!m_sessionId.empty())
        json.put("sessionId", m_sessionId);
    if (!m_instanceId.empty())
        json.put("instanceId", m_instanceId);

    json.put("appId", m_appId);
    json.put("displayId", m_displayId);
    json.put("type", m_type);
    json.put("status", m_status);
    json.put("pid", m_pid);
    json.put("time", m_time);
}
