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

#ifndef MEMSTAY_H_
#define MEMSTAY_H_

#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <vector>

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>

using namespace std;

class MemStay {
public:
    static MemStay& getInstance()
    {
        static MemStay instance;
        return instance;
    }

    virtual ~MemStay();
    void setInterval(int interval);
    void setUnit(int unit);
    void setMemUsageRate(float memUsageRate);
    void setSwapUsageRate(float swapUsageRate);

    void configure();

private:
    static gboolean _tick(gpointer data);
    static gboolean _tick_1sec(gpointer data);

    MemStay();
    void print(long currentSwap, long currentMem);

    long m_allocationSize;
    vector<void*> m_allocations;
    guint32 m_interval;
    int m_timer;
    int m_unit;

    int m_allocMemBlockCnt;
    int m_occupiedMemBlockCnt;
    int m_targetMemBlockCnt;
    int m_targetMemUsageBlock;
    float m_targetMemUsageRate;
    int m_totalMemBlock;

    int m_allocSwapBlockCnt;
    int m_occupiedSwapBlockCnt;
    int m_targetSwapBlockCnt;
    int m_targetSwapUsageBlock;
    float m_targetSwapUsageRate;
    int m_totalSwapBlock;
};

#endif /* MEMSTAY_H_ */
