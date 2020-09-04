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

#include "Process.h"
#include "util/Logger.h"

Process::Process()
    : m_ppid(-1),
      m_tid(-1),
      m_cmd(""),
      m_size(-1),
      m_rss(-1),
      m_shared(-1),
      m_text(-1),
      m_data(-1)
{
    setClassName("Process");
}

Process::~Process()
{
}

void Process::fromProc(proc_t& processInfo)
{
    if (m_ppid != -1 && m_ppid != processInfo.ppid) {
        Logger::error("Invalid request", getClassName());
        return;
    }
    m_ppid = processInfo.ppid;
    m_tid = processInfo.tid;
    m_cmd = processInfo.cmd;
    m_size = processInfo.size;
    m_rss = processInfo.rss;
    m_shared = processInfo.share;
    m_text = processInfo.trs;
    m_data = processInfo.drs;
}

void Process::setPpid(int pid)
{
    m_ppid = pid;
}

int Process::getPpid()
{
    return m_ppid;
}

void Process::setTid(int tid)
{
    m_tid = tid;
}

int Process::getTid() const
{
    return m_tid;
}

void Process::setCmd(string cmd)
{
    m_cmd = cmd;
}

string& Process::getCmd()
{
    return m_cmd;
}


void Process::setSize(int size)
{
    m_size = size;
}

int Process::getSize()
{
    return m_size;
}

void Process::setShared(int shared)
{
    m_shared = shared;
}

void Process::setRss(int rss)
{
    m_rss = rss;
}

int Process::getRss() const
{
    return m_rss;
}

int Process::getPss() const
{
    return m_rss - m_shared;
}

int Process::getShared()
{
    return m_shared;
}

void Process::setText(int text)
{
    m_text = text;
}

int Process::getText()
{
    return m_text;
}

void Process::setData(int data)
{
    m_data = data;
}

int Process::getData()
{
    return m_data;
}

void Process::print()
{
    Logger::verbose("PPID - " + to_string(m_ppid), getClassName());
    Logger::verbose("TID - " + to_string(m_tid), getClassName());
    Logger::verbose("CMD - " + m_cmd, getClassName());
    Logger::verbose("SIZE - " + to_string(m_size), getClassName());
    Logger::verbose("RSS - " + to_string(m_rss), getClassName());
    Logger::verbose("SHARED - " + to_string(m_shared), getClassName());
    Logger::verbose("TEXT - " + to_string(m_text), getClassName());
    Logger::verbose("DATA - " + to_string(m_data), getClassName());
}

void Process::print(JValue& json)
{
    // No implmeneted yet
}
