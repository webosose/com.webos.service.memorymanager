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

#ifndef MEMORYMANAGER_H_
#define MEMORYMANAGER_H_

#include <iostream>
#include <glib.h>

#include "luna/LunaManager.h"
#include "luna/client/ApplicationManager.h"
#include "memoryinfo/MemoryInfoManager.h"
#include "setting/SettingManager.h"

using namespace std;

class MemoryManager : public SettingManagerListener,
                      public LunaManagerListener,
                      public MemoryInfoManagerListener,
                      public ApplicationManagerListener {
public:
    static MemoryManager& getInstance()
    {
        static MemoryManager s_instance;
        return s_instance;
    }

    virtual ~MemoryManager();

    void initialize();
    void run();

    // MemoryManager
    virtual void onTick();

    // LunaManagerListener
    virtual bool onRequireMemory(int requiredMemory, string& errorText);
    virtual bool onManagerStatus(JValue& responsePayload);
    virtual bool onMemoryStatus(JValue& responsePayload);

    // MemoryInfoManagerListener
    virtual void onEnter(enum MemoryLevel prev, enum MemoryLevel cur);
    virtual void onLow();
    virtual void onCritical();

    // ApplicationManagerListener
    virtual void onApplicationsChanged();

private:
    static gboolean tick(gpointer user_data)
    {
        MemoryManager::getInstance().onTick();
        return G_SOURCE_CONTINUE;
    }

    MemoryManager();

    GMainLoop* m_mainloop;
    guint m_tickSrc;
    bool m_lock;

};

#endif /* CORE_SERVICE_MEMORYMANAGER_H_ */
