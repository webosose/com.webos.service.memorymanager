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

#ifndef BASE_APPLICATION_H_
#define BASE_APPLICATION_H_

#include <iostream>
#include <algorithm>
#include <pbnjson.hpp>

#include "Process.h"
#include "base/IPrintable.h"
#include "util/Time.h"

using namespace std;
using namespace pbnjson;

enum WindowType {
    WindowType_Unknown,
    WindowType_Card,
    WindowType_Overlay,
};

enum ApplicationType {
    ApplicationType_Unknown,
    ApplicationType_WebApp,
    ApplicationType_Native,
    ApplicationType_Qml,
};

enum ApplicationStatus {
    ApplicationStatus_Unknown,
    ApplicationStatus_Preload,
    ApplicationStatus_Background,
    ApplicationStatus_Foreground,
};

class Application : public IPrintable {
public:
    static string toString(enum WindowType& type);
    static void toEnum(string str, enum WindowType& type);

    static string toString(enum ApplicationType& type);
    static void toEnum(string str, enum ApplicationType& type);

    static string toString(enum ApplicationStatus& type);
    static void toEnum(string str, enum ApplicationStatus& type);

    static bool compare(const Application& a, const Application& b)
    {
        if (a.m_applicationStatus == b.m_applicationStatus) {
            if (a.m_applicationType == b.m_applicationType)
                return a.m_time > b.m_time;
            else
                return a.m_applicationType > b.m_applicationType;
        } else {
            return a.m_applicationStatus > b.m_applicationStatus;
        }
    }

    Application();
    Application(string appId);
    virtual ~Application();

    // setter / getter
    void setAppId(string appId)
    {
        m_appId = appId;
    }

    string getAppId() const
    {
        return m_appId;
    }

    void setInstanceId(string instanceId)
    {
        m_instanceId = instanceId;
    }

    string getInstanceId() const
    {
        return m_instanceId;
    }

    void setPid(int pid)
    {
        m_pid = pid;
    }

    int getPid() const
    {
        return m_pid;
    }

    void setWindowType(enum WindowType type)
    {
        m_windowType = type;
    }

    enum WindowType getWindowType()
    {
        return m_windowType;
    }

    void setApplicationType(enum ApplicationType type)
    {
        m_applicationType = type;
    }

    enum ApplicationType getApplicationType()
    {
        return m_applicationType;
    }

    void setApplicationStatus(enum ApplicationStatus status)
    {
        if (status == ApplicationStatus_Foreground) {
            updateTime();
        }
        m_applicationStatus = status;
    }

    enum ApplicationStatus getApplicationStatus()
    {
        return m_applicationStatus;
    }

    void updateTime()
    {
        m_time = Time::getSystemTime();
    }

    int getContext()
    {
        return m_context;
    }

    void setContext(int context)
    {
        m_context = context;
    }

    // IPrintable
    virtual void print();
    virtual void print(JValue& json);

private:
    string m_appId;
    string m_instanceId;
    int m_pid;

    enum WindowType m_windowType;
    enum ApplicationType m_applicationType;
    enum ApplicationStatus m_applicationStatus;

    Process m_process;

    // runtime value
    int m_time;
    int m_context;

};

#endif /* BASE_APPLICATION_H_ */
