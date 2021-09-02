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

#ifndef MEMORYMANAGER_H_
#define MEMORYMANAGER_H_

#include <iostream>
#include <glib.h>
#include <pbnjson.hpp>

#include "MMBus.h"
#include "memorymonitor/MemoryMonitor.h"
#include "luna2/LunaConnector.h"
#include "base/Runtime.h"
#include "session/Session.h"

#include "interface/IClassName.h"
#include "interface/ISingleton.h"
#include "interface/IPrintable.h"

using namespace std;
using namespace pbnjson;

class MemroyManager;

class MemoryLevel : public IClassName {
public:
    explicit MemoryLevel() = default;
    virtual ~MemoryLevel() {}

    virtual string toString() = 0;
    virtual void action(string& errorText) = 0;
    virtual bool keepLevel(long memAvail) = 0;

private:
};

class MemoryLevelNormal : public MemoryLevel {
public:
    explicit MemoryLevelNormal();
    virtual ~MemoryLevelNormal() {}

    virtual string toString() override final;
    virtual void action(string& errorText) override final;
    virtual bool keepLevel(long memAvail) override final;
};

class MemoryLevelLow : public MemoryLevel {
public:
    explicit MemoryLevelLow();
    virtual ~MemoryLevelLow() {}

    virtual string toString() override final;
    virtual void action(string& errorText) override final;
    virtual bool keepLevel(long memAvail) override final;
};

class MemoryLevelCritical : public MemoryLevel {
public:
    explicit MemoryLevelCritical();
    virtual ~MemoryLevelCritical() {}

    virtual string toString() override final;
    virtual void action(string& errorText) override final;
    virtual bool keepLevel(long memAvail) override final;
};

class MemoryManager : public ISingleton<MemoryManager>,
                      public IClassName,
                      public IPrintable {
public:
    explicit MemoryManager();
    virtual ~MemoryManager();

    void run();
#ifdef SUPPORT_LEGACY_API
    const string& getOldServiceName() const { return m_oldServiceName; }
#endif
    const string& getServiceName() const { return m_serviceName; }
    GMainLoop* getMainLoop() const { return m_mainLoop; }
    SessionMonitor& getSessionMonitor() const { return *m_sessionMonitor; }

    /* Handle insternal events */
    void handleMemoryMonitorEvent(MonitorEvent& event);
    void handleRuntimeChange(const string& appId, const string& instanceId,
                             const enum RuntimeChange& change);

    /* for exposed APIs used by LunaServiceProvider */
    bool onRequireMemory(const int requiredMemory, string& errorText);
    void onSysInfo(JValue& json);

    // IPrintable
    virtual void print() override final {};
    virtual void print(JValue& printOut) override final;

    // DBus
    bool registerSignal();

private:
    static bool onMemoryPressured(MMBusComWebosMemoryManager1 *object, guint var);

    LunaServiceProvider* m_lunaServiceProvider;
    GMainLoop* m_mainLoop;
    MemoryLevel* m_memoryLevel;
    MemoryMonitor* m_memoryMonitor;
#ifdef SUPPORT_LEGACY_API
    static const string m_oldServiceName;
#endif
    MMBusComWebosMemoryManager1* m_proxy;
    static const string m_serviceName;
    SessionMonitor* m_sessionMonitor;
};

#endif /* CORE_SERVICE_MEMORYMANAGER_H_ */
