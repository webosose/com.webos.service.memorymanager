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
#include <sys/mman.h>

#include "MemStay.h"
#include "util/Proc.h"

MemStay::MemStay()
    : m_allocationSize(0),
      m_allocMemBlockCnt(0),
      m_allocSwapBlockCnt(0),
      m_occupiedMemBlockCnt(0),
      m_occupiedSwapBlockCnt(0),
      m_interval(0),
      m_timer(-1),
      m_targetMemBlockCnt(0),
      m_targetMemUsageBlock(0),
      m_targetMemUsageRate(-1),
      m_targetSwapBlockCnt(0),
      m_targetSwapUsageBlock(0),
      m_targetSwapUsageRate(-1),
      m_totalMemBlock(0),
      m_totalSwapBlock(0),
      m_unit(0)
{
}

MemStay::~MemStay()
{
}

void MemStay::setInterval(int interval)
{
    m_interval = interval;
}

void MemStay::setUnit(int unit)
{
    m_unit = unit;
}

void MemStay::setMemUsageRate(float memUsageRate)
{
    m_targetMemUsageRate = memUsageRate;
}

void MemStay::setSwapUsageRate(float swapUsageRate)
{
    m_targetSwapUsageRate = swapUsageRate;
}

void MemStay::configure()
{
    long targetMemBlock;
    long totalMem = 0, freeMem = 0;
    long freeMemBlock = 0;

    long targetSwapBlock = 0;
    long totalSwap = 0, freeSwap = 0;
    long freeSwapBlock = 0;

    map<string, string> mInfo;

    Proc::getMemInfo(mInfo);

    auto it = mInfo.find("SwapTotal");
    if (it != mInfo.end()) {
        totalSwap = stol(it->second) / 1024;
    }
    it = mInfo.find("SwapFree");
    if (it != mInfo.end()) {
        freeSwap = stol(it->second) / 1024;
    }

    it = mInfo.find("MemTotal");
    if (it != mInfo.end()) {
        totalMem = stol(it->second) / 1024;
    }

    it = mInfo.find("MemAvailable");
    if (it != mInfo.end()) {
        freeMem = stol(it->second) / 1024;
    }

    if (m_targetSwapUsageRate > 0) {
        m_totalSwapBlock = totalSwap / m_unit; // MB based on m_unit
        freeSwapBlock = freeSwap / m_unit; // MB
        m_targetSwapUsageBlock = m_totalSwapBlock * m_targetSwapUsageRate;
        m_occupiedSwapBlockCnt = m_totalSwapBlock - freeSwapBlock;
        m_allocSwapBlockCnt = m_occupiedSwapBlockCnt;
    }
    if (m_targetMemUsageRate > 0) {

        m_totalMemBlock = totalMem / m_unit; //MB
        freeMemBlock = freeMem / m_unit;

        m_targetMemUsageBlock = m_totalMemBlock * m_targetMemUsageRate;
        m_occupiedMemBlockCnt  = m_totalMemBlock - freeMemBlock;
        m_allocMemBlockCnt  = m_occupiedMemBlockCnt;
    }


    cout << "[memstay] Total SwapUsageBlock : " << m_totalSwapBlock << endl;
    cout << "[memstay] Free SwapBlock : " << freeSwapBlock << endl;
    cout << "[memstay] Target SwapUsageBlock : " << m_targetSwapUsageBlock << endl;
    cout << "[memstay] OccupiedSwapblockCnt : " << m_occupiedSwapBlockCnt << endl;

    cout << "[memstay] Total MemBlock : " << m_totalMemBlock << endl;
    cout << "[memstay] Free MemBlock : " << freeMemBlock << endl;
    cout << "[memstay] Target MemUsageBlock : " << m_targetMemUsageBlock << endl;
    cout << "[memstay] OccupiedMemBlockCnt : " << m_occupiedMemBlockCnt << endl;

    cout << "[memstay] Unit             : " << m_unit << "MB" << endl;
    if ((m_timer = g_timeout_add(m_interval, _tick, NULL)) <= 0) {
        cerr << "[memstay] g_timeout_add fails" << endl;
    }
}

gboolean MemStay::_tick_1sec(gpointer data)
{
    long freeMem = 0;
    long freeSwap = 0;
    map<string, string> mInfo;
    long totalMem = 0;
    long totalSwap = 0;

    Proc::getMemInfo(mInfo);

    auto it = mInfo.find("SwapTotal");
    if (it != mInfo.end()) {
        totalSwap = stol(it->second) / 1024;
    }

    it = mInfo.find("SwapFree");
    if (it != mInfo.end()) {
        freeSwap = stol(it->second) / 1024;
    }

    it = mInfo.find("MemTotal");
    if (it != mInfo.end()) {
        totalMem = stol(it->second) / 1024;
    }

    it = mInfo.find("MemAvailable");
    if (it != mInfo.end()) {
        freeMem = stol(it->second) / 1024;
    }

    int size = MemStay::getInstance().m_unit * 1024 * 1024;
    void* buffer = NULL;

    MemStay::getInstance().print( (totalSwap == 0)? 0 : (100 * (totalSwap - freeSwap) / totalSwap), (totalMem == 0)? 0 : (100 * (totalMem - freeMem) / totalMem));

    return G_SOURCE_CONTINUE;
}

gboolean MemStay::_tick(gpointer data)
{
    static int memDone = 0;
    long freeMem = 0;
    long freeSwap = 0;
    map<string, string> mInfo;
    static int swapDone = 0;
    long totalMem = 0;
    long totalSwap = 0;

    Proc::getMemInfo(mInfo);

    auto it = mInfo.find("SwapTotal");
    if (it != mInfo.end()) {    
        totalSwap = stol(it->second) / 1024;
    }
    
    it = mInfo.find("SwapFree");    
    if (it != mInfo.end()) {
        freeSwap = stol(it->second) / 1024;
    }

    it = mInfo.find("MemTotal");
    if (it != mInfo.end()) {
        totalMem = stol(it->second) / 1024;
    }

    it = mInfo.find("MemAvailable");
    if (it != mInfo.end()) {    
        freeMem = stol(it->second) / 1024;
    }

    int size = MemStay::getInstance().m_unit * 1024 * 1024;
    void* buffer = NULL;

    if (swapDone == 0 && (MemStay::getInstance().m_targetSwapUsageRate > 0) 
                      && (MemStay::getInstance().m_allocSwapBlockCnt < MemStay::getInstance().m_targetSwapUsageBlock)) {
        buffer = mmap(NULL, size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (buffer == MAP_FAILED) {
            cerr << "[memstay] Allocation fails" << endl;
            return TRUE;
        }

        if (memset(buffer, 1, size) == NULL) {
            cerr << "[memstay] memset fails" << endl;
            return TRUE;
        }
        madvise(buffer, size, MADV_PAGEOUT);
        MemStay::getInstance().m_allocSwapBlockCnt++;

        MemStay::getInstance().print((totalSwap == 0)? 0 : (100 * (totalSwap - freeSwap) / totalSwap),(totalMem == 0)? 0 : (100 * (totalMem - freeMem) / totalMem));
        MemStay::getInstance().m_allocationSize += MemStay::getInstance().m_unit;
        MemStay::getInstance().m_allocations.push_back(buffer);

        return G_SOURCE_CONTINUE;
    } else {
        swapDone = 1;
    }

    if ((MemStay::getInstance().m_targetMemUsageRate > 0) && (MemStay::getInstance().m_allocMemBlockCnt < MemStay::getInstance().m_targetMemUsageBlock)) {
        buffer = mmap(NULL, size, PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED, -1, 0);

        if (buffer == MAP_FAILED) {
            cerr << "[memstay] Allocation fails" << endl;
            return TRUE;
        }

        if (memset(buffer, 1, size) == NULL) {
            cerr << "[memstay] memset fails" << endl;
            return TRUE;
        }
        MemStay::getInstance().m_allocMemBlockCnt++;
        MemStay::getInstance().m_allocationSize += MemStay::getInstance().m_unit;
        MemStay::getInstance().m_allocations.push_back(buffer);
    } else {
        memDone = 1;
    }

    MemStay::getInstance().print((totalSwap == 0)? 0 : (100 * (totalSwap-freeSwap) / totalSwap), (totalMem == 0)? 0: (100 * (totalMem-freeMem) / totalMem));

    if (memDone && swapDone) {
        if (g_timeout_add(1000, _tick_1sec, NULL) <= 0) {
            cerr << "[memstay] 1second g_timeout_add fails" << endl;
        }
        cout << "Complete!" << endl;
        return FALSE;
    }

    return G_SOURCE_CONTINUE;
}

void MemStay::print(long currentSwap, long currentMem)
{
    printf("[memstay] : swap-total(%d%%/%ld%%), swap-local(%dMB/%dMB), memory-total(%d%%/%ld%%), memory-local(%dMB/%dMB)\n",
             ((m_targetSwapUsageRate > 0) ? (int)(m_targetSwapUsageRate * 100) : 0), currentSwap,
             (m_targetSwapUsageBlock - m_occupiedSwapBlockCnt) * m_unit, (m_allocSwapBlockCnt - m_occupiedSwapBlockCnt) * m_unit,
             ((m_targetMemUsageRate > 0) ? (int)(m_targetMemUsageRate * 100) : 0),  currentMem,
             (m_targetMemUsageBlock - m_occupiedMemBlockCnt) * m_unit, (m_allocMemBlockCnt - m_occupiedMemBlockCnt) * m_unit);
}
