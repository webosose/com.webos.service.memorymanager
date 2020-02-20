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

#include "SettingManager.h"
#include "util/Logger.h"

#include <glib.h>
#include <strings.h>
#include <unistd.h>
#include <pbnjson.h>

#include "Environment.h"

#define LOG_NAME "SettingManager"

//const char* SettingManager::DEFAULT_CONFIG_FILE = "/etc/palm/memorymanager.json";
const char* SettingManager::DEFAULT_CONFIG_FILE = WEBOS_INSTALL_WEBOS_SYSCONFDIR "/memorymanager.json";

SettingManager::SettingManager()
    : m_configuration(Object())
{
    if (access(DEFAULT_CONFIG_FILE, R_OK) == 0) {
        loadSetting(DEFAULT_CONFIG_FILE);
    }
}

SettingManager::~SettingManager()
{
}

void SettingManager::initialize(GMainLoop* mainloop)
{

}

int SettingManager::getLowEnter()
{
    if (strcmp(WEBOS_TARGET_DISTRO, "webos") == 0) {
        return 250;
    } else {
        return 100;
    }
}

int SettingManager::getLowExit()
{
    if (strcmp(WEBOS_TARGET_DISTRO, "webos") == 0) {
        return 280;
    } else {
        return 120;

    }
}

int SettingManager::getCriticalEnter()
{
    if (strcmp(WEBOS_TARGET_DISTRO, "webos") == 0) {
        return 100;
    } else {
        return 50;

    }
}

int SettingManager::getCriticalExit()
{
    if (strcmp(WEBOS_TARGET_DISTRO, "webos") == 0) {
        return 130;
    } else {
        return 70;
    }
}

int SettingManager::getDefaultRequiredMemory()
{
    return DEFAULT_REQUIRED_MEMORY;
}

int SettingManager::getRetryCount()
{
    return DEFAULT_RETRY_COUNT;
}

bool SettingManager::isVerbose()
{
    return true;
}

bool SettingManager::isSingleAppPolicy()
{
    if (strcmp(WEBOS_TARGET_DISTRO, "webos") == 0 ||
        strcmp(WEBOS_TARGET_DISTRO, "webos-auto") == 0) {
        return false;
    }
    return true;
}

bool SettingManager::setSetting(JValue& source, JValue& local)
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
            setSetting(value, v);
        }
    }
    return true;
}

JValue SettingManager::getSetting(initializer_list<const char*> list)
{
    JValue* pos = &m_configuration;
    JValue result;
    for (auto iter = list.begin() ; iter != list.end() ; ++iter) {
        if (!pos->hasKey(*iter)) {
            return nullptr;
        } else {
            result = (*pos)[(*iter)];
            pos = &result;
        }
    }
    return result;
}

bool SettingManager::loadSetting(const string filename)
{
    JValue value = JDomParser::fromFile(filename.c_str());
    if (!value.isValid() || value.isNull()) {
        Logger::error("Fail Invalid Json formmated file " + filename, LOG_NAME);
        return false;
    }
    return setSetting(value, m_configuration);
}

string SettingManager::getSwapMode()
{
    JValue value = m_configuration["swap"]["mode"];
    if (value.isNull()) {
        return "";
    }
    return value.asString();
}

string SettingManager::getSwapPartition()
{
    JValue value = m_configuration["swap"]["partition"];
    if (value.isNull()) {
        return "";
    }
    return value.asString();
}

int SettingManager::getSwapSize()
{
    JValue value = m_configuration["swap"]["size"];
    if (value.isNull()) {
        return 0;
    }
    return value.asNumber<int32_t>();
}
