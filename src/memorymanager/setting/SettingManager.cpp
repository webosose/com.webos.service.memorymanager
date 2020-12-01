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

#include "SettingManager.h"
#include "util/Logger.h"

#include <glib.h>
#include <strings.h>
#include <unistd.h>
#include <pbnjson.h>

#include "Environment.h"

bool SettingManager::m_SingleAppPolicy;
bool SettingManager::m_SessionEnabled;

int SettingManager::m_memoryLevelLowEnter;
int SettingManager::m_memoryLevelLowExit;
int SettingManager::m_memoryLevelCriticalEnter;
int SettingManager::m_memoryLevelCriticalExit;

void SettingManager::initEnv()
{
    char* ls2EnableSession = getenv("LS2_ENABLE_SESSION");

    if (ls2EnableSession != NULL && strcmp(ls2EnableSession, "true") == 0)
        m_SessionEnabled = true;
    else
        m_SessionEnabled = false;

    if (strcmp(WEBOS_TARGET_DISTRO, "webos") == 0 ||
        strcmp(WEBOS_TARGET_DISTRO, "webos-auto") == 0) {
        m_SingleAppPolicy = false;

        m_memoryLevelLowEnter = 250;
        m_memoryLevelLowExit = 280;
        m_memoryLevelCriticalEnter = 100;
        m_memoryLevelCriticalExit = 130;
    } else {
        m_SingleAppPolicy = true;

        m_memoryLevelLowEnter = 100;
        m_memoryLevelLowExit = 120;
        m_memoryLevelCriticalEnter = 50;
        m_memoryLevelCriticalExit = 70;
    }
}

int SettingManager::getMemoryLevelLowEnter()
{
    return m_memoryLevelLowEnter;
}

int SettingManager::getMemoryLevelLowExit()
{
    return m_memoryLevelLowExit;
}

int SettingManager::getMemoryLevelCriticalEnter()
{
    return m_memoryLevelCriticalEnter;
}

int SettingManager::getMemoryLevelCriticalExit()
{
    return m_memoryLevelCriticalExit;
}

bool SettingManager::getSingleAppPolicy()
{
    return m_SingleAppPolicy;
}

bool SettingManager::getSessionEnabled()
{
    return m_SessionEnabled;
}

int SettingManager::loadSetting()
{
    // From build environment
    initEnv();

    return 0;
}
