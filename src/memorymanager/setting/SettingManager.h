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

#ifndef SETTING_SETTINGMANAGER_H_
#define SETTING_SETTINGMANAGER_H_

#include <iostream>
#include <pbnjson.hpp>

using namespace std;
using namespace pbnjson;

class SettingManager {
public:
    static int loadSetting();

    // From build environment
    static bool getSessionEnabled();
    static bool getSingleAppPolicy();

    static int getMemoryLevelLowEnter();
    static int getMemoryLevelLowExit();
    static int getMemoryLevelCriticalEnter();
    static int getMemoryLevelCriticalExit();

private:
    SettingManager() = delete;
    virtual ~SettingManager();

    static void initEnv();

    // From build environmena
    static bool m_SingleAppPolicy;
    static bool m_SessionEnabled;

    static int m_memoryLevelLowEnter;
    static int m_memoryLevelLowExit;
    static int m_memoryLevelCriticalEnter;
    static int m_memoryLevelCriticalExit;
};

#endif /* SETTING_SETTINGMANAGER_H_ */
