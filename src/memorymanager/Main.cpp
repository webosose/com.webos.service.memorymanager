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

#include <iostream>
#include <glib.h>

#include "MemoryManager.h"
#include "setting/SettingManager.h"
#include "util/Logger.h"

#define LOG_NAME "MemoryManager_MAIN"

using namespace std;

int main(int argc, char** argv)
{
    if (SettingManager::loadSetting() < 0) {
        Logger::error("Fail to load SettingManager", LOG_NAME);
        return 0;
    }
    Logger::normal("SettingManager Initialized", LOG_NAME);

    if (MemoryManager::getInstance().initialize() < 0) {
        return 0;
    }

    MemoryManager::getInstance().run();
    Logger::normal("Exit main of MemoryManager", LOG_NAME);
    return 0;
}

