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

#include "interface/IClassName.h"
#include "interface/ISingleton.h"
#include "interface/IPrintable.h"

#include "memorymonitor/MemoryMonitor.h"
#include "luna/LunaManager.h"

using namespace std;
using namespace pbnjson;

class MemoryLevel : public IClassName {
public:
    MemoryLevel() {}
    virtual ~MemoryLevel() {}

    virtual string toString() = 0;
    virtual void action() = 0;
    virtual bool keepLevel(long memAvail) = 0;
};

class MemoryLevelNormal : public MemoryLevel { 
public:
    MemoryLevelNormal();
    virtual ~MemoryLevelNormal() {}

    virtual string toString();
    virtual void action();
    virtual bool keepLevel(long memAvail);
};

class MemoryLevelLow : public MemoryLevel { 
public:
    MemoryLevelLow();
    virtual ~MemoryLevelLow() {}

    virtual string toString();
    virtual void action();
    virtual bool keepLevel(long memAvail);
};

class MemoryLevelCritical : public MemoryLevel {
public:
    MemoryLevelCritical();
    virtual ~MemoryLevelCritical() {}

    virtual string toString();
    virtual void action();
    virtual bool keepLevel(long memAvail);
};

class MemoryManager : public ISingleton<MemoryManager>,
                      public IClassName,
                      public IPrintable,
                      public LunaManagerListener {
public:
    MemoryManager();
    virtual ~MemoryManager();

    void run();
    void handleMemoryMonitorEvent(MonitorEvent& event);
    GMainLoop* getMainLoop();

    // IPrintable
    virtual void print() {};
    virtual void print(JValue& json);

    // LunaManagerListener
    virtual bool onRequireMemory(int requiredMemory, string& errorText);
    virtual bool onManagerStatus(JValue& responsePayload);
    virtual void onMemoryStatus(JValue& responsePayload);

private:
    static const int m_retryCount = 5;
    GMainLoop* m_mainLoop;

    MemoryMonitor* m_memoryMonitor;
    MemoryLevel* m_memoryLevel;
};

#endif /* CORE_SERVICE_MEMORYMANAGER_H_ */
