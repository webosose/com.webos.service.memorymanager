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

#include "MemoryInfoManager.h"

#include "setting/SettingManager.h"
#include "util/Logger.h"
#include "util/Proc.h"

#define LOG_NAME    "ProcMeminfo"

string MemoryInfoManager::toString(enum MemoryLevel level)
{
    switch (level) {
    case MemoryLevel_NORMAL:
        return "normal";

    case MemoryLevel_LOW:
        return "low";

    case MemoryLevel_CRITICAL:
        return "critical";
    }
    return "unknown";
}


MemoryInfoManager::MemoryInfoManager()
    : m_total(0)
    , m_free(0)
    , m_level(MemoryLevel_NORMAL)
{
}

MemoryInfoManager::~MemoryInfoManager()
{
}

void MemoryInfoManager::initialize(GMainLoop* mainloop)
{
}

void MemoryInfoManager::update(bool disableCallback)
{
    Proc::getMemoryInfo(m_total, m_free);

    // update current level
    enum MemoryLevel prevLevel = m_level;
    if (m_free < SettingManager::getInstance().getCriticalEnter()) {
        m_level = MemoryLevel_CRITICAL;
    } else if (m_free < SettingManager::getInstance().getCriticalExit()) {
        if (prevLevel == MemoryLevel_CRITICAL)  m_level = MemoryLevel_CRITICAL;
        else                                    m_level = MemoryLevel_LOW;
    } else if (m_free < SettingManager::getInstance().getLowEnter()) {
        m_level = MemoryLevel_LOW;
    } else if (m_free < SettingManager::getInstance().getLowExit()) {
        if (prevLevel == MemoryLevel_LOW)  m_level = MemoryLevel_LOW;
        else                               m_level = MemoryLevel_NORMAL;
    } else {
        m_level = MemoryLevel_NORMAL;
    }

    if (disableCallback || m_listener == nullptr)
        return;

    // notify level change
    if (m_level != prevLevel) {
        switch(m_level) {
        case MemoryLevel_NORMAL:
            m_listener->onEnter(prevLevel, MemoryLevel_NORMAL);
            break;

        case MemoryLevel_LOW:
            m_listener->onEnter(prevLevel, MemoryLevel_LOW);
            break;

        case MemoryLevel_CRITICAL:
            m_listener->onEnter(prevLevel, MemoryLevel_CRITICAL);
            break;
        }
    }

    switch(m_level) {
    case MemoryLevel_LOW:
        m_listener->onLow();
        break;

    case MemoryLevel_CRITICAL:
        m_listener->onCritical();
        break;

    default:
        break;
    }
}

enum MemoryLevel MemoryInfoManager::getCurrentLevel()
{
    return m_level;
}

enum MemoryLevel MemoryInfoManager::getExpectedLevel(int requiredMemory)
{
    static char buffer[256];
    long expectedAvailable = m_free - requiredMemory;

    int result;
    result = sprintf(buffer, "Free(%ld) - RequiredMemory(%d) = ExpectedMemory(%ld)", m_free, requiredMemory, expectedAvailable);
    if (result > 0) {
        Logger::normal(buffer, LOG_NAME);
    } else {
        Logger::warning("Failed in sprintf", LOG_NAME);
    }

    if (expectedAvailable < SettingManager::getInstance().getCriticalEnter()) {
        return MemoryLevel_CRITICAL;
    } else if (m_free < SettingManager::getInstance().getLowEnter()) {
        return MemoryLevel_LOW;
    } else {
        return MemoryLevel_NORMAL;
    }
}

void MemoryInfoManager::print()
{
    // TODO
}

void MemoryInfoManager::print(JValue& json)
{
    JValue current = pbnjson::Object();
    current.put("level", toString(m_level));
    current.put("total", (int)m_total);
    current.put("free", (int)m_free);
    json.put("system", current);

    JValue low = pbnjson::Object();
    low.put("enter", SettingManager::getInstance().getLowEnter());
    low.put("exit", SettingManager::getInstance().getLowExit());

    JValue critical = pbnjson::Object();
    critical.put("enter", SettingManager::getInstance().getCriticalEnter());
    critical.put("exit", SettingManager::getInstance().getCriticalExit());

    JValue threshold = pbnjson::Object();
    threshold.put("low", low);
    threshold.put("critical", critical);
    json.put("threshold", threshold);
}
