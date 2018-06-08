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
