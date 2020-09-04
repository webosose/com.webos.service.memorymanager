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

#include "interface/IManager.h"
#include "interface/ISingleton.h"

using namespace std;
using namespace pbnjson;

class SettingManager : public ISingleton<SettingManager> {
friend class ISingleton<SettingManager>;
public:
    virtual ~SettingManager();

    // IManager
    void initialize(GMainLoop* mainloop);

    bool loadSetting(const string filename);

    int getLowEnter();
    int getLowExit();
    int getCriticalEnter();
    int getCriticalExit();

    int getDefaultRequiredMemory();
    int getRetryCount();

    bool isVerbose();
    bool isSingleAppPolicy();
    bool isSessionEnabled()
    {
        return m_isSessionEnabled;
    }

    bool setSetting(JValue& value, JValue& local);
    JValue getSetting(initializer_list<const char*> list);
    string getSwapMode();
    string getSwapPartition();
    int getSwapSize();

private:
    static const int DEFAULT_RETRY_COUNT = 5;
    static const int DEFAULT_REQUIRED_MEMORY = 120;

    SettingManager();

    static const string DEFAULT_CONFIG_FILE;

    JValue m_configuration;
    bool m_isSessionEnabled;
};

#endif /* SETTING_SETTINGMANAGER_H_ */
