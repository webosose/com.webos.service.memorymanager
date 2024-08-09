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

#include "MemoryMonitor.h"
#include "MemoryManager.h"

#include "util/Logger.h"
#include "util/Proc.h"

void AvailMemMonitor::initSource(GMainLoop* loop)
{
    GMainContext* gCtxt = g_main_loop_get_context(loop);
    gpointer gptr = (gpointer)this;

    m_source = g_timeout_source_new_seconds(m_updatePeriod);
    g_source_set_callback(m_source, MonitorEvent::onSourceCallback, gptr, NULL);
    m_sourceId = g_source_attach(m_source, gCtxt);
}

void AvailMemMonitor::deinitSource()
{
    g_source_destroy(m_source);
    g_source_unref(m_source);
}

void AvailMemMonitor::update(void)
{
    map<string, string> mInfo;

    Proc::getMemInfo(mInfo);

    auto it = mInfo.find("MemTotal");
    if (it != mInfo.end()) {
        m_total = stoul(it->second) / 1024;
    }

    it = mInfo.find("MemAvailable");
    if (it != mInfo.end()) {
        m_available = stoul(it->second) / 1024;
    }

    this->m_memoryMonitor.raiseEvent((MonitorEvent&)*this);
}

long AvailMemMonitor::getAvailable(void)
{
    return m_available;
}

AvailMemMonitor::AvailMemMonitor(MemoryMonitor& monitor, GMainLoop* loop)
    : m_memoryMonitor(monitor)
{
    initSource(loop);
}

AvailMemMonitor::~AvailMemMonitor()
{
    deinitSource();
}

void MemoryMonitor::raiseEvent(MonitorEvent& e)
{
    MemoryManager *mm = MemoryManager::getInstance();

    mm->handleMemoryMonitorEvent(e);
}

MemoryMonitor::MemoryMonitor()
{
    setClassName("MemoryMonitor");
    MemoryManager *mm = MemoryManager::getInstance();
    MonitorEvent *e;

    /* Create list of monitor event */
    e = new AvailMemMonitor(*this, mm->getMainLoop());
    m_eventList.push_front(e);
}

MemoryMonitor::~MemoryMonitor()
{
    while (!m_eventList.empty()) {
        MonitorEvent *e = m_eventList.front();
        delete e;
        m_eventList.pop_front();
    }
}
