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

#ifndef BASE_APPLICATION_H_
#define BASE_APPLICATION_H_

#include <iostream>
#include <algorithm>
#include <pbnjson.hpp>

#include "interface/IClassName.h"
#include "interface/IPrintable.h"

#include "util/Time.h"

using namespace std;
using namespace pbnjson;

class Application : public IPrintable,
                    public IClassName {
public:
    Application();
    virtual ~Application();

    static bool isCloseable(const string& status);
    static int getStatusPriority(const string& status);
    static int getTypePriority(const string& type);
    static bool compare(const Application& a, const Application& b);

    bool isCloseable();

    void setProperty(string instanceId, string appId, string type, int pid, int displayId);
    void setStatus(const string& status);

    const string& getInstanceId() const { return m_instanceId; }
    const string& getAppId() const { return m_appId; }
    const string& getType() const { return m_type; }
    int getDisplayId() const { return m_displayId; }
    int getPid() const { return m_pid; }
    const string& getStatus() const { return m_status; }

    // IPrintable
    virtual void print();
    virtual void print(JValue& json);


private:
    /* Managed by SAM::onRunning */
    string m_instanceId;    /* unique id of application */
    string m_appId;         /* name of application */
    string m_type;          /* type of application (web, native, ...) */
    int m_displayId;           /* unique display id where application is launched */
    int m_pid;              /* pid of non-web type application */

    /* Managed by SAM::onGetAppLifeEvents */
    string m_status;        /* status or event of application (FG, BG, ...) */
    int m_time;             /* timestamp for LRU policy among FG applications */
};

#endif /* BASE_APPLICATION_H_ */
