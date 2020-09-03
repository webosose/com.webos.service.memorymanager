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
#include "setting/SettingManager.h"
#include "swap/SwapManager.h"
#include "luna/client/SAM.h"

#include "util/Logger.h"

MemoryManager::MemoryManager()
    : m_tickSrc(-1),
      m_lock(false),
      m_retry_count(5)
{
    setClassName("MemoryManager");
    m_mainloop = g_main_loop_new(NULL, FALSE);

    LunaManager::getInstance().initialize(m_mainloop);
    Logger::normal("Initialized LunaManager", getClassName());
    MemoryInfoManager::getInstance().initialize(m_mainloop);
    Logger::normal("Initialized MemoryInfoManager", getClassName());
    SwapManager::getInstance().initialize(m_mainloop);
    Logger::normal("Initialized SwapManager", getClassName());

    LunaManager::getInstance().setListener(this);
    MemoryInfoManager::getInstance().setListener(this);
}

MemoryManager::~MemoryManager()
{
    g_main_loop_unref(m_mainloop);
}

void MemoryManager::run()
{
    MemoryInfoManager::getInstance().update(false);

    m_tickSrc = g_timeout_add_seconds(1, tick, this);

    Logger::normal("Start mainLoop", getClassName());
    g_main_loop_run(m_mainloop);
}

bool MemoryManager::onRequireMemory(int requiredMemory, string& errorText)
{
    if (SettingManager::getSingleAppPolicy()) {
        Logger::normal("SingleApp Policy. Skipping memory level check", getClassName());
        return true;
    }

    for (int i = 0; i < m_retry_count; ++i) {
        if (MemoryInfoManager::getInstance().getExpectedLevel(requiredMemory) != MemoryLevel_CRITICAL) {
            return true;
        }

        if (!SAM::close(true, errorText)) {
            return false;
        }

        // TODO Need to find better way to check memory level again.
        // because it takes time to free allocated memory after closing application
        MemoryInfoManager::getInstance().update();
    }
    errorText = "Failed to reclaim required memory. Timeout.";
    return false;
}

void MemoryManager::onMemoryStatus(JValue& responsePayload)
{
    MemoryInfoManager::getInstance().print(responsePayload);
    SAM::toJson(responsePayload);
}

bool MemoryManager::onManagerStatus(JValue& responsePayload)
{
    return true;
}

void MemoryManager::onEnter(enum MemoryLevel prev, enum MemoryLevel cur)
{
    LunaManager::getInstance().postMemoryStatus();
    LunaManager::getInstance().signalLevelChanged(MemoryInfoManager::toString(prev), MemoryInfoManager::toString(cur));

    switch (cur) {
    case MemoryLevel_NORMAL:
        Logger::normal("MemoryLevel - NORMAL", getClassName());
        break;

    case MemoryLevel_LOW:
        Logger::normal("MemoryLevel - LOW", getClassName());
        break;

    case MemoryLevel_CRITICAL:
        Logger::normal("MemoryLevel - CRITICAL", getClassName());
        break;
    }
}

void MemoryManager::onLow()
{
    if (m_lock)
        return;
    m_lock = true;
    string errorText = "";
    if (!SAM::close(false, errorText)) {
        Logger::error(errorText, getClassName());
    }
    m_lock = false;
}

void MemoryManager::onCritical()
{
    if (m_lock)
        return;
    m_lock = true;
    string errorText = "";
    if (!SAM::close(true, errorText)) {
        Logger::error(errorText, getClassName());
    }
    m_lock = false;
}
