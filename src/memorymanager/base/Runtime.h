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
    explicit BaseProcess() = default;
    virtual ~BaseProcess() { }

    long getPssValue(const int pid);

    virtual void updateMemStat() = 0;
};

class Application : public BaseProcess,
                    public IPrintable,
                    public IClassName {
public:
    explicit Application(const string& instanceId, const string& appId,
                         const string& type, const string& status,
                         const int pid);
    virtual ~Application() { }

    bool isCloseable();

    void setStatus(const string& status);
    void setType(const string& type);
    void setPid(const int pid);

    const string& getInstanceId() const { return m_instanceId; }
    const string& getAppId() const { return m_appId; }
    const string& getStatus() const { return m_status; }

    bool operator==(const Application& compare);

    virtual void updateMemStat() override final;

    // IPrintable
    virtual void print() override final;
    virtual void print(JValue& json) override final;

private:
    const string m_instanceId;  // unique id of application
    const string m_appId;       // name of application
    string m_type;              // type of application (web, native, ...)
    string m_status;            // status or event of application (FG, BG, ...)
    int m_pid;                  // Linux PID
    unsigned long m_pssKb;      // PSS in KB size
};

class Service : public BaseProcess,
                public IPrintable,
                public IClassName {
public:
    explicit Service(const string& serviceId, const list<int>& pid);
    virtual ~Service() { }

    const string& getServiceId() const { return m_serviceId; }
    map<int, unsigned long>& getPidPss() { return m_pidPss; }

    template<typename T, typename U>
    void toString(map<T, U>& pMap, string& str1, string& str2);

    virtual void updateMemStat() override final;

    // IPrintable
    virtual void print() override final;
    virtual void print(JValue& json) override final;

private:
    const string m_serviceId;           // name of service
    map<int, unsigned long> m_pidPss;   // Linux PID and PSS in KB size,
                                        // Service Class may have multiple PIDs and PSS,
                                        // While Application has only one PID and PSS.
};

enum class RuntimeChange : char {
    APP_UPDATE = 0,
    APP_ADD,
    APP_REMOVE,
    APP_CLOSE,
};

class Runtime : public IClassName {
public:
    explicit Runtime(Session& session);
    virtual ~Runtime();

    void updateMemStat();
    bool reclaimMemory(bool critical);

    /* Reserved Pid List Management */
    void clearReservedPid();
    void addReservedPid(const int pid);

    /* Service List Management */
    void addService(Service* service);
    void updateService(const string& serviceId, const int pid);
    void createService(Session *sesssion);
    int countService();
    void printService();
    void printService(JValue& json);

    /* Application List Management */
    bool updateApp(const string& appId, const string& instanceId,
                   const string& event);
    void addApp(Application& app);
    int countApp();
    list<Application>::reverse_iterator findFirstForeground();
    const string findFirstForegroundAppId();
    void printApp();
    void printApp(JValue& json);
    void setAppDefaultStatus(const string& foregroundAppId);

private:
    static const string WAM_SERVICE_ID;
    static const string SAM_SERVICE_ID;

    Session& m_session;

    list<int> m_reservedPids;
    list<Service*> m_services;
    list<Application> m_applications;
};

#endif /* BASE_RUNTIME_H_ */
