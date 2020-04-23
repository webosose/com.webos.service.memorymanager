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

#include "util/JValueUtil.h"
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
    if (m_sessionId.empty())
        sprintf(buffer , "%-30s %15s %10s %10d %10d", m_appId.c_str(), m_status.c_str(), m_type.c_str(), m_pid, m_time);
    else
        sprintf(buffer , "%-40s %-30s %15s %10s %10d %10d", m_sessionId.c_str(), m_appId.c_str(), m_status.c_str(), m_type.c_str(), m_pid, m_time);
    Logger::verbose(buffer, "Application");
}

void Application::print(JValue& json)
{
    if (!m_sessionId.empty())
        JValueUtil::putValue(json, "sessionId", m_sessionId);
    if (!m_instanceId.empty())
        JValueUtil::putValue(json, "instanceId", m_instanceId);

    JValueUtil::putValue(json, "appId", m_appId);
    JValueUtil::putValue(json, "displayId", m_displayId);
    JValueUtil::putValue(json, "type", m_type);
    JValueUtil::putValue(json, "status", m_status);
    JValueUtil::putValue(json, "pid", m_pid);
    JValueUtil::putValue(json, "time", m_time);
}
