/* @@@LICENSE
 *
 * Copyright (c) 2018 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */

#include "MemStay.h"
#include "util/Proc.h"

MemStay::MemStay()
    : m_target(0)
    , m_interval(0)
    , m_unit(0)
    , m_free(false)
    , m_allocationSize(0)
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
    g_timeout_add(m_interval, _tick, NULL);
}

gboolean MemStay::_tick(gpointer data)
{
    long totalMemory;
    long freeMemory;

    if (!Proc::getMemoryInfo(totalMemory, freeMemory))
        return G_SOURCE_CONTINUE;

    int size = MemStay::getInstance().m_unit * 1024 * 1024;
    void* buffer = NULL;

    if (freeMemory > MemStay::getInstance().m_target && freeMemory - MemStay::getInstance().m_unit > 0) {
        buffer = malloc(size);
        if (buffer == NULL) {
            cerr << "[memstay] Allocation Fails" << endl;
            return TRUE;
        }
        memset(buffer, 1, size);

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
