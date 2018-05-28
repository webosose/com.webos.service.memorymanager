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

#ifndef MEMSTAY_H_
#define MEMSTAY_H_

#include <fstream>
#include <iostream>
#include <vector>

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
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

    static int getFreeMemory();

    virtual ~MemStay();

    void setTarget(int target);
    void setInterval(int interval);
    void setUnit(int unit);
    void setFree(bool free);

    void configure();

private:
    static gboolean _tick(gpointer data);

    MemStay();

    void print(char type, long available);

    int m_target;
    guint32 m_interval;
    int m_unit;
    bool m_free;

    vector<void*> m_allocations;
    long m_allocationSize;
};

#endif /* MEMSTAY_H_ */
