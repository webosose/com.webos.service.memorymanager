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

#ifndef BASE_RUNTIME_H_
#define BASE_RUNTIME_H_

#include <iostream>
#include <list>

#include <session/Session.h>

#include "interface/IClassName.h"
#include "interface/IPrintable.h"

using namespace std;

class BaseProcess {
public:
    BaseProcess();
    virtual ~BaseProcess() { }

    BaseProcess(int pid);

    void setPid(const int pid);
    void updateMemStat();

    int m_pid;              // Linux PID
    unsigned long m_pssKb;  // PSS in KB size
};

class Application : public BaseProcess,
                    public IPrintable,
                    public IClassName {
public:
    Application();
    virtual ~Application() { }

    bool isCloseable();

    Application(string instanceId, string appId, string status);
    Application(string instanceId, string appId, string type, string status, int pid);
    void setStatus(const string& status);
    void setType(const string& type);

    const string& getInstanceId() const { return m_instanceId; }
    const string& getAppId() const { return m_appId; }
    const string& getStatus() const { return m_status; }

    bool operator==(const Application& compare);

    // IPrintable
    virtual void print();
    virtual void print(JValue& json);

private:
    string m_instanceId;    // unique id of application
    string m_appId;         // name of application
    string m_type;          // type of application (web, native, ...)
    string m_status;        // status or event of application (FG, BG, ...)
};

class Service : public BaseProcess,
                public IPrintable,
                public IClassName {
public:
    Service();
    virtual ~Service() { }

    Service(string serviceId, int pid);

    const string& getServiceId();

    // IPrintable
    virtual void print();
    virtual void print(JValue& json);

private:
    string m_serviceId;     // name of service
};

enum RuntimeChange {
    RUNTIME_CHANGE_APP_UPDATE = 0,
    RUNTIME_CHANGE_APP_ADD,
    RUNTIME_CHANGE_APP_REMOVE,
    RUNTIME_CHANGE_APP_CLOSE,
};

class Runtime : public IClassName {
public:
    Runtime();
    virtual ~Runtime() { }

    Runtime(Session& session);

    void updateMemStat();
    bool reclaimMemory(bool critical);

    /* Service List Management */
    void addService(Service& service);
    int countService();
    void printService();
    void printService(JValue& json);

    /* Application List Management */
    bool updateApp(const string& appId, const string& event);
    void addApp(Application& app);
    int countApp();
    list<Application>::reverse_iterator findFirstForeground();
    void printApp();
    void printApp(JValue& json);

private:
    Session& m_session;

    list<Service> m_services;
    list<Application> m_applications;
};

#endif /* BASE_RUNTIME_H_ */
