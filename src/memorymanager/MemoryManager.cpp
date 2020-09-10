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
#include "luna2/LunaConnector.h"
#include "session/Session.h"
#include "setting/SettingManager.h"
#include "swap/SwapManager.h"

#include "util/Logger.h"
#include "util/Proc.h"

#include <map>

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

void MemoryLevelNormal::action(string& errorText)
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

void MemoryLevelLow::action(string& errorText)
{
    MemoryManager* mm = MemoryManager::getInstance();
    auto sessions = mm->getSessionMonitor().getSessions();
    int allAppCount = 0;

    for (auto it = sessions.begin(); it != sessions.end(); it++) {
        it->second->reclaimMemory(false);
        allAppCount += it->second->getSessionAppCount();
    }

    if (allAppCount == 0) {
        errorText = "Failed to reclaim required memory. All apps were closed";
    }

    #if 0
    bool reclaimed;
    string errorText = "";

    LunaManager::getInstance().postManagerKillingEvent(application);

    if (!closed) {
        errorText = "Failed to reclaim required memory. All apps were closed";
        Logger::error(errorText, getClassName());
    }
    #endif
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

void MemoryLevelCritical::action(string& errorText)
{
    MemoryManager* mm = MemoryManager::getInstance();
    auto sessions = mm->getSessionMonitor().getSessions();
    int allAppCount = 0;

    for (auto it = sessions.begin(); it != sessions.end(); it++) {
        it->second->reclaimMemory(true);
        allAppCount += it->second->getSessionAppCount();
    }

    if (allAppCount == 0) {
        errorText = "Failed to reclaim required memory. All apps were closed";
    }

#if 0
    bool closed;
    string errorText = "";

    closed = SAM::close(true);
    // LunaManager::getInstance().postManagerKillingEvent(application);

    if (!closed) {
        errorText = "Failed to reclaim required memory. All apps were closed";
        Logger::error(errorText, getClassName());
    }
#endif
}

const string MemoryManager::m_serviceName = "com.webos.service.memorymanager";

void MemoryManager::run()
{
    m_memoryLevel = new MemoryLevelNormal;

    m_memoryMonitor = new MemoryMonitor();
    Logger::normal("MemoryMonitor Initialized", getClassName());

    m_lunaServiceProvider = new LunaServiceProvider();
    Logger::normal("LunaServiceProvider Initialized", getClassName());

    m_sessionMonitor = new SessionMonitor();
    Logger::normal("SessionMonitor Initialized", getClassName());

    SwapManager::getInstance().initialize(m_mainLoop);
    Logger::normal("SwapManager Initialized ", getClassName());

    Logger::normal("Start mainLoop", getClassName());
    g_main_loop_run(m_mainLoop);
}

void MemoryManager::handleMemoryMonitorEvent(MonitorEvent& event)
{
    MemoryLevel *prev;
    long memAvail;
    string errorText = "";

    // Currently we use AvailMemMonitor only
    if (typeid(event) != typeid(AvailMemMonitor))
        return;

    AvailMemMonitor& m = static_cast<AvailMemMonitor&>(event);
    memAvail = m.getAvailable();

    /* MemoryLevel changed */
    if (!m_memoryLevel->keepLevel(memAvail)) {
        prev = m_memoryLevel;

        if (memAvail < SettingManager::getMemoryLevelCriticalEnter())
            m_memoryLevel = new MemoryLevelCritical;
        else if (memAvail < SettingManager::getMemoryLevelLowEnter())
            m_memoryLevel = new MemoryLevelLow;
        else
            m_memoryLevel = new MemoryLevelNormal;

        Logger::normal("MemoryLevel changed from " + prev->toString() +
                       "to " + m_memoryLevel->toString(), getClassName());

        m_lunaServiceProvider->postMemoryStatus();
        m_lunaServiceProvider->raiseSignalLevelChanged(prev->toString(), m_memoryLevel->toString());

        delete prev;
    }

    m_memoryLevel->action(errorText);
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

    JValue apps = pbnjson::Array();

    auto sessions = m_sessionMonitor->getSessions();

    for (auto it = sessions.begin(); it != sessions.end(); it++)
        it->second->printApplications(apps);

    json.put("applications", apps);
}

bool MemoryManager::onRequireMemory(int requiredMemory, string& errorText)
{
    MemoryLevel *level = NULL;
    long total, available;
    int i, requested;
    bool ret = false;

    if (SettingManager::getSingleAppPolicy()) {
        Logger::normal("SingleAppPolicy, Skip memory level check", getClassName());
        return true;
    }

    if (requiredMemory <= 0)
        requested = m_defaultRequiredMemory;
    else
        requested = requiredMemory;

    Proc::getMemoryInfo(total, available);

    if (available - requested > SettingManager::getMemoryLevelCriticalEnter())
        return true;

    level = new MemoryLevelCritical;
    for (i = 0; i < m_retryCount; i++) {
        level->action(errorText);
        /* TODO : wait progess... */

        Proc::getMemoryInfo(total, available);
        if (available - requested > SettingManager::getMemoryLevelCriticalEnter()) {
            ret = true;
            break;
        }
    }
    delete level;
    return ret;
}

void MemoryManager::killApplication(Application& app)
{
    m_lunaServiceProvider->postManagerKillingEvent(app);
}

void MemoryManager::renewMemoryStatus()
{
    m_lunaServiceProvider->postMemoryStatus();
}

MemoryManager::MemoryManager()
{
    GMainContext* gCtxt;

    setClassName("MemoryManager");

    gCtxt = g_main_context_new();
    m_mainLoop = g_main_loop_new(gCtxt, FALSE);
}

MemoryManager::~MemoryManager()
{
    g_main_loop_unref(m_mainLoop);

    delete m_memoryMonitor;
    delete m_lunaServiceProvider;
}
