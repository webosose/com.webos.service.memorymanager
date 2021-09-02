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

#include <chrono>
#include <map>
#include <thread>

#include "MMBus.h"
#include "setting/SettingManager.h"
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
}

#ifdef SUPPORT_LEGACY_API
const string MemoryManager::m_oldServiceName = "com.webos.memorymanager";
#endif
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

void MemoryManager::print(JValue& printOut)
{
    int total, available;

    /* Get Meminfo */
    map<string, string> mInfo;
    Proc::getMemInfo(mInfo);

    auto it = mInfo.find("MemTotal");
    total = stoi(it->second) / 1024;
    it = mInfo.find("MemAvailable");
    available = stoi(it->second) / 1024;

    /* Organize "system" */
    JValue current = pbnjson::Object();
    current.put("level", m_memoryLevel->toString());
    current.put("total", total);
    current.put("available", available);
    printOut.put("system", current);

    /* Organize "threshold" */
    JValue threshold = pbnjson::Object();

    JValue low = pbnjson::Object();
    low.put("enter", SettingManager::getMemoryLevelLowEnter());
    low.put("exit", SettingManager::getMemoryLevelLowExit());

    JValue critical = pbnjson::Object();
    critical.put("enter", SettingManager::getMemoryLevelCriticalEnter());
    critical.put("exit", SettingManager::getMemoryLevelCriticalExit());

    threshold.put("low", low);
    threshold.put("critical", critical);
    printOut.put("threshold", threshold);

    /* Organize "applications" */
    JValue apps = pbnjson::Array();
    printOut.put("applications", apps);
    auto sessions = m_sessionMonitor->getSessions();
    for (auto it = sessions.cbegin(); it != sessions.cend(); ++it) {
        if (it->second->m_runtime->countApp() > 0)
            it->second->m_runtime->printApp(apps);
    }
}

void MemoryManager::handleRuntimeChange(const string& appId, const string& instanceId,
                                        const enum RuntimeChange& change)
{
    if (change == RuntimeChange::APP_CLOSE)
        m_lunaServiceProvider->postManagerEventKilling(appId, instanceId);
    else
        m_lunaServiceProvider->postMemoryStatus();
}

bool MemoryManager::onMemoryPressured(MMBusComWebosMemoryManager1 *object, guint var)
{
    const int type_swap = 0, type_psi = 1;
    if (var == type_psi) { // we will handle PSI only
        MemoryManager* self = MemoryManager::getInstance();
        auto sessions = self->getSessionMonitor().getSessions();
        int allAppCount = 0;

        auto it = sessions.cbegin();
        if (it != sessions.cend()) {
            it->second->m_runtime->reclaimMemory(true);
            allAppCount += it->second->m_runtime->countApp();
            Logger::normal("reclaimMemory called by PSI : allApp" + std::to_string(allAppCount));
        }
        if (allAppCount == 0) {
            Logger::normal("Failed to reclaim required memory. No more app to be closed");
        }
    } else if (var == type_swap) {// we will ignore TYPE_SWAP now
        Logger::normal("Received SWAP Pressure");
    }

    return true;
}

bool MemoryManager::onRequireMemory(const int requiredMemory, string& errorText)
{
    return true;
}

void MemoryManager::onSysInfo(JValue& json)
{
    auto sessions = getSessionMonitor().getSessions();
    JValue sessionList = pbnjson::Array();

    for (auto it = sessions.cbegin(); it != sessions.cend(); ++it) {
        JValue session = pbnjson::Object();
        JValue appList = pbnjson::Array();
        JValue serviceList = pbnjson::Array();

        /*
         * TODO : Can we move this code during initialization?
         * We want to create service list when session runtime is created once.
         * But, we do not know when all systemd services' initialization id done.
         * We tried to find this moment by using systemctl :
         * http://gpro.lge.com/c/webosose/com.webos.service.memorymanager/+/294155
         * This, however, brings another issue that app cannot be launched
         * until MM initialization is done.
         */
        it->second->m_runtime->createService(it->second);

        it->second->m_runtime->updateMemStat();
        it->second->m_runtime->printApp(appList);
        it->second->m_runtime->printService(serviceList);

        session.put("sessionId", it->second->getSessionId());
        session.put("apps", appList);
        session.put("services", serviceList);
        sessionList.append(session);
    }

    json.put("sessions", sessionList);

    /* For debugging, print session, service list, and app list */
    for (auto it = sessions.cbegin(); it != sessions.cend(); ++it) {
        it->second->print();

        if (it->second->m_runtime->countService() > 0)
            it->second->m_runtime->printService();

        if (it->second->m_runtime->countApp() > 0)
            it->second->m_runtime->printApp();
    }
}

bool MemoryManager::registerSignal()
{
    GDBusConnection *conn;
    GError *error = NULL;
    guint arg1;

    conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
    if (error != NULL) {
        Logger::normal("Failed to get bus", getClassName());
        g_error_free(error);
        return false;
    }

    m_proxy = mmbus_com_webos_memory_manager1_proxy_new_sync(
                                  conn,
                                  G_DBUS_PROXY_FLAGS_NONE,
                                  "com.webos.MemoryManager1",
                                  "/com/webos/MemoryManager1",
                                  NULL,
                                  &error);

    if (m_proxy == NULL) {
        Logger::normal("Failed to create proxy", getClassName());
        g_error_free(error);
        return false;
    }

    g_signal_connect(m_proxy, "memory-pressured", G_CALLBACK(MemoryManager::onMemoryPressured), NULL);
    Logger::normal("DBus Signal registered", getClassName());

    return true;
}

MemoryManager::MemoryManager()
{
    setClassName("MemoryManager");

    m_mainLoop = g_main_loop_new(NULL, FALSE);

    m_memoryLevel = nullptr;
    m_memoryMonitor = nullptr;
    m_sessionMonitor = nullptr;
    m_lunaServiceProvider = nullptr;

    if (registerSignal() == false)
        Logger::normal("Failed to register dbus signal", getClassName());
}

MemoryManager::~MemoryManager()
{
    g_main_loop_unref(m_mainLoop);

    g_object_unref(m_proxy);
    delete m_memoryMonitor;
    delete m_lunaServiceProvider;
}
