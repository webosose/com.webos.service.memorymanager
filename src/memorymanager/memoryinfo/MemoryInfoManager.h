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

#ifndef MEMORYINFO_MEMORYINFOMANAGER_H_
#define MEMORYINFO_MEMORYINFOMANAGER_H_

#include <iostream>
#include <fstream>

#include "interface/IManager.h"
#include "interface/ISingleton.h"
#include "interface/IPrintable.h"

using namespace std;

enum MemoryLevel {
    MemoryLevel_NORMAL = 0,
    MemoryLevel_LOW,
    MemoryLevel_CRITICAL
};

class MemoryInfoManagerListener {
public :
    MemoryInfoManagerListener() {};
    virtual ~MemoryInfoManagerListener() {};

    virtual void onEnter(enum MemoryLevel prev, enum MemoryLevel cur) = 0;
    virtual void onLow() = 0;
    virtual void onCritical() = 0;
};

class MemoryInfoManager : public IManager<MemoryInfoManagerListener>,
                          public ISingleton<MemoryInfoManager>,
                          public IPrintable {
friend class ISingleton<MemoryInfoManager>;
public:
    static string toString(enum MemoryLevel level);

    virtual ~MemoryInfoManager();

    void initialize(GMainLoop* mainloop);

    void update(bool disableCallback = true);

    enum MemoryLevel getCurrentLevel();
    enum MemoryLevel getExpectedLevel(int memory);

    virtual void print();
    virtual void print(JValue& json);

private:
    MemoryInfoManager();

    long m_total;
    long m_free;
    enum MemoryLevel m_level;

};

#endif /* MEMORYINFO_MEMORYINFOMANAGER_H_ */
