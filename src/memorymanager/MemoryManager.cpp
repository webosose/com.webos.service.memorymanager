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

#include "MemoryManager.h"

#include "memorymonitor/MemoryMonitor.h"
#include "setting/SettingManager.h"
#include "swap/SwapManager.h"
#include "luna/client/SAM.h"

#include "util/Logger.h"
#include "util/Proc.h"

MemoryLevelNormal::MemoryLevelNormal()
{
    setClassName("MemoryLevelNormal");
}

string MemoryLevelNormal::toString()
{
        return "normal";
}

bool MemoryLevelNormal::keepLevel(long memAvail)
{
    if (memAvail > SettingManager::getMemoryLevelLowEnter())
        return true;
    else
        return false;
}

void MemoryLevelNormal::action()
{

}

MemoryLevelLow::MemoryLevelLow()
{
    setClassName("MemoryLevelLow");
}

string MemoryLevelLow::toString()
{
        return "low";
}

bool MemoryLevelLow::keepLevel(long memAvail)
{
    if (memAvail < SettingManager::getMemoryLevelLowExit() &&
        memAvail > SettingManager::getMemoryLevelCriticalEnter())
        return true;
    else
        return false;
}

void MemoryLevelLow::action()
{
    bool closed;
    string errorText = "";

    closed = SAM::close(false, errorText);

    if (!closed)
        Logger::error(errorText, getClassName());
}

MemoryLevelCritical::MemoryLevelCritical()
{
    setClassName("MemoryLevelCritical");
}

string MemoryLevelCritical::toString()
{
        return "critical";
}

bool MemoryLevelCritical::keepLevel(long memAvail)
{
    if (memAvail < SettingManager::getMemoryLevelCriticalExit())
        return true;
    else 
        return false;
}

void MemoryLevelCritical::action()
{
    bool closed;
    string errorText = "";

    closed = SAM::close(true, errorText);

    if (!closed)
        Logger::error(errorText, getClassName());
}

MemoryManager::MemoryManager()
{
    GMainContext* gCtxt;

    setClassName("MemoryManager");

    gCtxt = g_main_context_new();
    m_mainLoop = g_main_loop_new(gCtxt, FALSE);

    m_memoryLevel = new MemoryLevelNormal;

    m_memoryMonitor = new MemoryMonitor(*this);
    Logger::normal("MemoryMonitor Initialized", getClassName());

    ///////////////////////////////////////////////////////////////////////
    LunaManager::getInstance().initialize(m_mainLoop);
    Logger::normal("Initialized LunaManager", getClassName());

    SwapManager::getInstance().initialize(m_mainLoop);
    Logger::normal("Initialized SwapManager", getClassName());

    LunaManager::getInstance().setListener(this);
}

MemoryManager::~MemoryManager()
{
    g_main_loop_unref(m_mainLoop);

    delete m_memoryMonitor;
}

GMainLoop* MemoryManager::getMainLoop()
{
    return m_mainLoop;
}

void MemoryManager::run()
{
    Logger::normal("Start mainLoop", getClassName());
    g_main_loop_run(m_mainLoop);
}

void MemoryManager::handleMemoryMonitorEvent(MonitorEvent& event)
{
    MemoryLevel *level, *tmp;
    long memAvail;

    // Currently we use AvailMemMonitor only
    if (typeid(event) != typeid(AvailMemMonitor))
        return;

    AvailMemMonitor& m = static_cast<AvailMemMonitor&>(event);
    memAvail = m.getAvailable();

    /* MemoryLevel changed */
    if (!m_memoryLevel->keepLevel(memAvail)) {
        if (memAvail < SettingManager::getMemoryLevelCriticalEnter())
            level = new MemoryLevelCritical;
        else if (memAvail < SettingManager::getMemoryLevelLowEnter())
            level = new MemoryLevelLow;
        else
            level = new MemoryLevelNormal;

        tmp = m_memoryLevel;
        m_memoryLevel = level;
        level = tmp;

        Logger::normal("MemoryLevel changed to " + m_memoryLevel->toString(),
                getClassName());

        LunaManager::getInstance().postMemoryStatus();
        LunaManager::getInstance().signalLevelChanged(level->toString(),
                m_memoryLevel->toString());

        delete level;
    }

    m_memoryLevel->action();
}

void MemoryManager::print(JValue& json)
{
    long total, available;

    Proc::getMemoryInfo(total, available);

    JValue current = pbnjson::Object();
    current.put("level", m_memoryLevel->toString());
    current.put("total", (int)total);
    current.put("available", (int)available);
    json.put("system", current);

    JValue low = pbnjson::Object();
    low.put("enter", SettingManager::getMemoryLevelLowEnter());
    low.put("exit", SettingManager::getMemoryLevelLowExit());

    JValue critical = pbnjson::Object();
    critical.put("enter", SettingManager::getMemoryLevelCriticalEnter());
    critical.put("exit", SettingManager::getMemoryLevelCriticalExit());

    JValue threshold = pbnjson::Object();
    threshold.put("low", low);
    threshold.put("critical", critical);
    json.put("threshold", threshold);
}

bool MemoryManager::onRequireMemory(int requiredMemory, string& errorText)
{
    MemoryLevel *level = NULL;
    long total, available;
    int i = 0;

    if (SettingManager::getSingleAppPolicy()) {
        Logger::normal("SingleAppPolicy, Skip memory level check", getClassName());
        return true;
    }

    while (i < m_retryCount) {
        Proc::getMemoryInfo(total, available);

        if (available - requiredMemory > SettingManager::getMemoryLevelCriticalEnter()) {
            if (level)
                delete level;
            return true;
        }

        if (!level)
            level = new MemoryLevelCritical;

        level->action();
        i++;
    }

    delete level;

    errorText = "Failed to reclaim required memory. Timeout.";
    return false;
}

void MemoryManager::onMemoryStatus(JValue& responsePayload)
{
    print(responsePayload);
    SAM::toJson(responsePayload);
}

bool MemoryManager::onManagerStatus(JValue& responsePayload)
{
    return true;
}
