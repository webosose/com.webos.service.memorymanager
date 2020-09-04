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

#ifndef MEMORYMANAGER_H_
#define MEMORYMANAGER_H_

#include <iostream>
#include <glib.h>

#include "luna/LunaManager.h"
#include "luna/client/SAM.h"
#include "luna/client/SessionManager.h"
#include "memoryinfo/MemoryInfoManager.h"
#include "setting/SettingManager.h"
#include "swap/SwapManager.h"

using namespace std;

class MemoryManager : public ISingleton<MemoryManager>,
                      public LunaManagerListener,
                      public MemoryInfoManagerListener {
friend class ISingleton<MemoryManager>;
public:
    virtual ~MemoryManager();

    void initialize();
    void run();

    // MemoryManager
    virtual void onTick();

    // LunaManagerListener
    virtual bool onRequireMemory(int requiredMemory, string& errorText);
    virtual bool onManagerStatus(JValue& responsePayload);
    virtual void onMemoryStatus(JValue& responsePayload);

    // MemoryInfoManagerListener
    virtual void onEnter(enum MemoryLevel prev, enum MemoryLevel cur);
    virtual void onLow();
    virtual void onCritical();

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
