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

#ifndef SETTING_SETTINGMANAGER_H_
#define SETTING_SETTINGMANAGER_H_

#include <iostream>

#include "base/IManager.h"

#define DEFAULT_LOW_EXIT          280
#define DEFAULT_LOW_ENTER         250
#define DEFAULT_CRITICAL_EXIT     130
#define DEFAULT_CRITICAL_ENTER    100

using namespace std;

class SettingManagerListener {
public:
    SettingManagerListener() {};
    virtual ~SettingManagerListener() {};

};

class SettingManager : public IManager<SettingManagerListener> {
public:
    static SettingManager& getInstance()
    {
        static SettingManager s_instance;
        return s_instance;

    }

    virtual ~SettingManager();

    // IManager
    void initialize(GMainLoop* mainloop);

    int getLowEnter();
    int getLowExit();
    int getCriticalEnter();
    int getCriticalExit();

    int getDefaultRequiredMemory();
    int getRetryCount();
    bool isVerbose();

private:
    SettingManager();

    int m_lowEnter;
    int m_lowExit;
    int m_criticalEnter;
    int m_criticalExit;
};

#endif /* SETTING_SETTINGMANAGER_H_ */
