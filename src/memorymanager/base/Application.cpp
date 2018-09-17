// Copyright (c) 2018 LG Electronics, Inc.
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

string Application::toString(enum WindowType& type)
{
    switch (type) {
    case WindowType_Card:
        return "card";

    case WindowType_Overlay:
        return "overlay";

    default:
        break;
    }
    return "unknown";
}

void Application::toEnum(string str, enum WindowType& type)
{
    if (str == "card" || str == "_WEBOS_WINDOW_TYPE_CARD") {
        type = WindowType_Card;
    } else if (str == "overlay" || str == "_WEBOS_WINDOW_TYPE_OVERLAY"){
        type = WindowType_Overlay;
    } else {
        type = WindowType_Unknown;
    }
}

string Application::toString(enum ApplicationType& type)
{
    switch (type) {
    case ApplicationType_WebApp:
        return "web";

    case ApplicationType_Native:
        return "native";

    case ApplicationType_Qml:
        return "qml";

    default:
        break;
    }
    return "unknown";
}

void Application::toEnum(string str, enum ApplicationType& type)
{
    if (str == "native" || str == "native_builtin") {
        type = ApplicationType_Native;
    } else if (str == "web") {
        type = ApplicationType_WebApp;
    } else if (str == "qml") {
        type = ApplicationType_Qml;
    } else {
        type = ApplicationType_Unknown;
    }
}

string Application::toString(enum ApplicationStatus& type)
{
    switch (type) {
    case ApplicationStatus_Preload:
        return "preload";

    case ApplicationStatus_Background:
        return "background";

    case ApplicationStatus_Foreground:
        return "foreground";

    default:
        break;
    }
    return "unknown";
}

void Application::toEnum(string str, enum ApplicationStatus& type)
{
    if (str == "foreground" || str == "launch")
        type = ApplicationStatus_Foreground;
    else if (str == "preload")
        type = ApplicationStatus_Preload;
    else if (str == "background")
        type = ApplicationStatus_Background;
    else
        type = ApplicationStatus_Unknown;
}

Application::Application()
    : m_appId("")
    , m_tid(0)
    , m_windowType(WindowType_Unknown)
    , m_applicationType(ApplicationType_Unknown)
    , m_applicationStatus(ApplicationStatus_Unknown)
    , m_time(0)
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
    string msg = "STATUS(" + toString(m_applicationStatus) + ") ";
    msg += "TIME(" + to_string(m_time) + ") ";
    msg += "PID(" + to_string(m_tid) + ") ";
    msg += "WINDOW(" + toString(m_windowType) + ") ";
    msg += "TYPE(" + toString(m_applicationType) + ")";

    Logger::verbose(msg, m_appId);
}

void Application::print(JValue& json)
{
    json.put("appId", m_appId);
    json.put("pid", m_tid);
    json.put("type", toString(m_applicationType));
    json.put("status", toString(m_applicationStatus));
    json.put("time", m_time);
}
