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

const string SettingManager::m_configFile = string(WEBOS_INSTALL_WEBOS_SYSCONFDIR) + "/memorymanager.json";
JValue SettingManager::m_config(Object());

bool SettingManager::m_SingleAppPolicy;
bool SettingManager::m_SessionEnabled;

int SettingManager::m_memoryLevelLowEnter;
int SettingManager::m_memoryLevelLowExit;
int SettingManager::m_memoryLevelCriticalEnter;
int SettingManager::m_memoryLevelCriticalExit;

void SettingManager::initConfig(JValue& source, JValue& local)
{
    auto it = source.children();

    for (auto object = it.begin() ; object != it.end() ; ++object) {
        string key = (*object).first.asString();
        JValue value = (*object).second;

        if (!local.hasKey(key)) {
            local.put(key, value);
        } else if (!value.isObject()){
            local.put(key, value);
        } else {
            JValue v = local[key];
            initConfig(value, v);
        }
    }
}

const string SettingManager::getSwapMode()
{
    JValue value = m_config["swap"]["mode"];
    if (value.isNull())
        return "";

    return value.asString();
}

const string SettingManager::getSwapPartition()
{
    JValue value = m_config["swap"]["partition"];
    if (value.isNull())
        return "";

    return value.asString();
}

const int SettingManager::getSwapSize()
{
    JValue value = m_config["swap"]["size"];
    if (value.isNull())
        return 0;

    return value.asNumber<int32_t>();
}

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
    // From target's memorymanager.json
    if (int ret = access(m_configFile.c_str(), R_OK); ret < 0)
        return ret;

    JValue config = JDomParser::fromFile(m_configFile.c_str());
    if (!config.isValid() || config.isNull())
        return -EINVAL;

    initConfig(config ,m_config);

    // From build environment
    initEnv();

    return 0;
}
