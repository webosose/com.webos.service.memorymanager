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

class Application : public IPrintable {
public:
    static int getStatusPriority(const string& status)
    {
        if (status == "stop")
            return 0;
        else if (status == "close")
            return 1;
        else if (status == "launch")
            return 2;
        else if (status == "foreground")
            return 3;
        else if (status == "background")
            return 4;
        else if (status == "preload")
            return 5;
        else
            return 6;
    }

    static int getTypePriority(const string& type)
    {
        if (type == "web")
            return 0;
        else if (type.find("native") != std::string::npos)
            return 1;
        else
            return 2;
    }
    static bool compare(const Application& a, const Application& b)
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

    Application();
    Application(string appId);
    virtual ~Application();

    // setter / getter
    void setAppId(const string& appId)
    {
        m_appId = appId;
    }

    const string& getAppId() const
    {
        return m_appId;
    }

    void setInstanceId(const string& instanceId)
    {
        m_instanceId = instanceId;
    }

    const string& getInstanceId() const
    {
        return m_instanceId;
    }

    void setType(const string& type)
    {
        m_type = type;
    }

    const string& getType()
    {
        return m_type;
    }

    void setStatus(const string& status)
    {
        if (status == "foreground") {
            updateTime();
        }
        m_status = status;
    }

    const string& getStatus()
    {
        return m_status;
    }

    void setDisplayId(const int displayId)
    {
        m_displayId = displayId;
    }

    const int getDisplayId() const
    {
        return m_displayId;
    }

    void setPid(int pid)
    {
        m_pid = pid;
    }

    int getPid() const
    {
        return m_pid;
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
    string m_type;
    string m_status;
    int m_displayId;
    int m_pid;

    Process m_process;

    // runtime value
    int m_time;
    int m_context;

};

#endif /* BASE_APPLICATION_H_ */
