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

#include <unistd.h>

#include "Environment.h"

SettingManager::SettingManager()
{
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

