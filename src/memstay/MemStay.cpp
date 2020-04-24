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

#include "MemStay.h"
#include "util/Proc.h"

MemStay::MemStay()
    : m_target(0),
      m_interval(0),
      m_unit(0),
      m_free(false),
      m_allocationSize(0)
{
}

MemStay::~MemStay()
{
}

void MemStay::setTarget(int target)
{
    m_target = target;
}

void MemStay::setInterval(int interval)
{
    m_interval = interval;
}

void MemStay::setUnit(int unit)
{
    m_unit = unit;
}

void MemStay::setFree(bool free)
{
    m_free = free;
}

void MemStay::configure()
{
    if (g_timeout_add(m_interval, _tick, NULL) <= 0) {
        cerr << "[memstay] g_timeout_add fails" << endl;
    }
}

gboolean MemStay::_tick(gpointer data)
{
    long totalMemory;
    long freeMemory;

    Proc::getMemoryInfo(totalMemory, freeMemory);

    int size = MemStay::getInstance().m_unit * 1024 * 1024;
    void* buffer = NULL;

    if (freeMemory > MemStay::getInstance().m_target && freeMemory - MemStay::getInstance().m_unit > 0) {
        buffer = malloc(size);
        if (buffer == NULL) {
            cerr << "[memstay] Allocation fails" << endl;
            return TRUE;
        }
        if (memset(buffer, 1, size) == NULL) {
            cerr << "[memstay] memset fails" << endl;
            return TRUE;
	}

        MemStay::getInstance().m_allocationSize += MemStay::getInstance().m_unit;
        MemStay::getInstance().m_allocations.push_back(buffer);
        MemStay::getInstance().print('+', freeMemory);
    } else if (MemStay::getInstance().m_free && (freeMemory + MemStay::getInstance().m_unit) < MemStay::getInstance().m_target) {
        void* freeBuffer = MemStay::getInstance().m_allocations.back();
        free(freeBuffer);

        MemStay::getInstance().m_allocationSize -= MemStay::getInstance().m_unit;
        MemStay::getInstance().m_allocations.pop_back();
        MemStay::getInstance().print('-', freeMemory);
    } else {
        MemStay::getInstance().print('=', freeMemory);
    }
    return G_SOURCE_CONTINUE;
}

void MemStay::print(char type, long available)
{
    cout << "[memstay] " << type << " : "
         << "Target(" << m_target << "MB) "
         << "Free("  << available << "MB) "
         << "Hold("  << m_allocationSize << "MB)"
         << endl;
}
