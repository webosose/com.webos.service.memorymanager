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

#include "MemoryManager.h"

#include "luna/client/ApplicationManager.h"
#include "util/Logger.h"

#define LOG_NAME "MemoryManager"

MemoryManager::MemoryManager()
    : m_tickSrc(-1)
    , m_lock(false)
{
    m_mainloop = g_main_loop_new(NULL, FALSE);
}

MemoryManager::~MemoryManager()
{
    g_main_loop_unref(m_mainloop);
}

void MemoryManager::initialize()
{
    SettingManager::getInstance().initialize(m_mainloop);
    LunaManager::getInstace().initialize(m_mainloop);
    MemoryInfoManager::getInstance().initialize(m_mainloop);

    SettingManager::getInstance().setListener(this);
    LunaManager::getInstace().setListener(this);
    MemoryInfoManager::getInstance().setListener(this);
    ApplicationManager::getInstance().setListener(this);
}

void MemoryManager::run()
{
    m_tickSrc = g_timeout_add_seconds(1, tick, this);
    g_main_loop_run(m_mainloop);
}

void MemoryManager::onTick()
{
    MemoryInfoManager::getInstance().update(false);
}

bool MemoryManager::onRequireMemory(int requiredMemory, string& errorText)
{
    for (int i = 0; i < SettingManager::getInstance().getRetryCount(); ++i) {
        if (MemoryInfoManager::getInstance().getExpectedLevel(requiredMemory) != MemoryLevel_CRITICAL) {
            return true;
        }

        if (ApplicationManager::getInstance().getRunningAppCount() == 0) {
            errorText = "Failed to reclaim required memory. All apps were closed";
            // TODO This should be reboot?
            return false;
        }

        ApplicationManager::getInstance().closeApp(true);
        MemoryInfoManager::getInstance().update();
    }
    errorText = "Failed to reclaim required memory. Timeout.";
    return false;
}

bool MemoryManager::onMemoryStatus(JValue& responsePayload)
{
    MemoryInfoManager::getInstance().print(responsePayload);
    ApplicationManager::getInstance().print(responsePayload);
    return true;
}

bool MemoryManager::onManagerStatus(JValue& responsePayload)
{
    return true;
}

void MemoryManager::onEnter(enum MemoryLevel prev, enum MemoryLevel cur)
{
    LunaManager::getInstace().postMemoryStatus();
    LunaManager::getInstace().signalLevelChanged(MemoryInfoManager::toString(prev), MemoryInfoManager::toString(cur));

    switch (cur) {
    case MemoryLevel_NORMAL:
        Logger::normal("MemoryLevel - NORMAL", LOG_NAME);
        break;

    case MemoryLevel_LOW:
        Logger::normal("MemoryLevel - LOW", LOG_NAME);
        break;

    case MemoryLevel_CRITICAL:
        Logger::normal("MemoryLevel - CRITICAL", LOG_NAME);
        break;
    }
}

void MemoryManager::onLow()
{
    if (m_lock)
        return;
    m_lock = true;
    ApplicationManager::getInstance().closeApp(false);
    m_lock = false;
}

void MemoryManager::onCritical()
{
    if (m_lock)
        return;
    m_lock = true;
    ApplicationManager::getInstance().closeApp(true);
    m_lock = false;
}

void MemoryManager::onApplicationsChanged()
{
    LunaManager::getInstace().postMemoryStatus();
}
