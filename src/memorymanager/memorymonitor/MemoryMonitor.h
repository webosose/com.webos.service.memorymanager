// Copyright (c) 2020 LG Electronics, Inc.
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

#ifndef MEMORYMONITOR_MEMORYMONITOR_H_
#define MEMORYMONITOR_MEMORYMONITOR_H_

#include <glib.h>
#include <iostream>
#include <fstream>
#include <list>

#include "interface/IClassName.h"

using namespace std;

class MemoryMonitor;

class MonitorEvent {
public:
    explicit MonitorEvent() = default;
    virtual ~MonitorEvent() {}

    virtual void initSource(GMainLoop* loop) {};
    virtual void deinitSource() {};
    virtual void update() {};

    static gboolean onSourceCallback(gpointer eventInstance)
    {
        MonitorEvent *e = static_cast<MonitorEvent *>(eventInstance);
        e->update();
        return G_SOURCE_CONTINUE;
    }

protected:
    GSource *m_source;
    int m_sourceId;
};

class AvailMemMonitor : public MonitorEvent {
public:
    explicit AvailMemMonitor(MemoryMonitor& monitor, GMainLoop* loop);
    virtual ~AvailMemMonitor();

    long getAvailable(void);

    // MonitorEvent
    virtual void initSource(GMainLoop* loop) override final;
    virtual void deinitSource() override final;
    virtual void update() override final;

private:
    static const int m_updatePeriod = 1;

    unsigned long m_total;
    unsigned long m_available;
    MemoryMonitor& m_memoryMonitor;
};

class MemoryMonitor : public IClassName {
public:
    explicit MemoryMonitor();
    virtual ~MemoryMonitor();

    void raiseEvent(MonitorEvent& event);

private:
    list<MonitorEvent> m_eventList;
};

#endif /* MEMORYMONITOR_MEMORYMONITOR_H_ */
