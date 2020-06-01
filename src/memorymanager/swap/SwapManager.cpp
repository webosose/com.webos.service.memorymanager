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
#include "SwapManager.h"
#include "setting/SettingManager.h"
#include "util/Logger.h"

#include <unistd.h>
#include <regex>

#include "Environment.h"

const string SwapManager::EFS_CTL_BIN = "efsctl";
const string SwapManager::EFS_MAPPER_PATH = "/dev/mapper/eswap";
const string SwapManager::EFS_PARTLABEL = "swap";

SwapManager::SwapManager()
    : m_mode(SwapMode_NO),
      m_partition(""),
      m_size(0)
{
    setClassName("SwapManager");
}

SwapManager::~SwapManager()
{
}

bool SwapManager::setMode(const string mode)
{
    const string no = "NO";
    const string memory = "MEMORY";
    const string full = "FULL";

    if (mode == no)
        m_mode = SwapMode_NO;
    else if (mode == memory)
        m_mode = SwapMode_MEMORY;
    else if (mode == full)
        m_mode = SwapMode_FULL;
    else
        return false;

    return true;
}

static inline void rtrim(string &s)
{
    s.erase(find_if(s.rbegin(), s.rend(), [](int ch) {
        return !isspace(ch);
    }).base(), s.end());
}

static string execCmd(const char* cmd)
{
    array<char, 4096> buffer;
    string result;
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

    if (!pipe || !pipe.get()) {
        throw runtime_error("Error: popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

string SwapManager::findPartitionByPartLabel(const string partLabel)
{
    string cmd = "ls -l /dev/disk/by-partlabel/";
    cmd += partLabel;
    string result = execCmd(cmd.c_str());
    rtrim(result);
    Logger::normal("[Success] result : " + result, getClassName());

    size_t found = result.find_last_of("/");   /* ... -> ../../sdaX */
    string partNum = result.substr(found + 1); /* sdaX */
    string partition = "/dev/" + partNum;      /* /dev/sdaX */

    return partition;
}

void SwapManager::setPartition(const string partition)
{
    if (!partition.empty()) {
        m_partition = partition;
        return;
    }

    /* Find default partition by partlabel */
    m_partition = findPartitionByPartLabel(EFS_PARTLABEL);
}

bool SwapManager::setSize(const int size)
{
    if (size < 0)
        return false;

    m_size = size;
    return true;
}

bool SwapManager::createEFS(const enum SwapMode mode, const string partition,
                            const int size)
{
    if (mode != SwapMode_MEMORY && mode != SwapMode_FULL) {
        Logger::error("Mode should be MEMORY or FULL!", getClassName());
        return false;
    }

    string cmd = EFS_CTL_BIN;
    cmd += " create -p " + partition;
    cmd += " -s " + to_string(size);
    if (mode == SwapMode_FULL)
        cmd += " -w"; /* Set shrink_enable (i.e. use secondary storage) */
    int ret = ::system(cmd.c_str());
    if (ret != 0) {
        Logger::error("cmd : " + cmd + ", ret : " + to_string(ret) + \
                      ", WEXITSTATUS(ret) : " + to_string(WEXITSTATUS(ret)), \
                      getClassName());
        return false;
    }

    return true;
}

bool SwapManager::createSwap(const string device)
{
    /* Make swapspace */
    string cmd = "mkswap " + device;
    int ret = ::system(cmd.c_str());
    if (ret != 0) {
        Logger::error("cmd : " + cmd + ", ret : " + to_string(ret) + \
                      ", WEXITSTATUS(ret) : " + to_string(WEXITSTATUS(ret)), \
                      getClassName());
        return false;
    }

    /* Make sure the format info has been physically written to swap file */
    cmd = "sync";
    ret = ::system(cmd.c_str());
    if (ret != 0) {
        Logger::error("cmd : " + cmd + ", ret : " + to_string(ret) + \
                      ", WEXITSTATUS(ret) : " + to_string(WEXITSTATUS(ret)), \
                      getClassName());
        return false;
    }

    /* Swapon swapspace */
    cmd = "swapon " + device;
    ret = ::system(cmd.c_str());
    if (ret != 0) {
        Logger::error("cmd : " + cmd + ", ret : " + to_string(ret) + \
                      ", WEXITSTATUS(ret) : " + to_string(WEXITSTATUS(ret)), \
                      getClassName());
        return false;
    }

    return true;
}

void SwapManager::initialize(GMainLoop* mainloop)
{
    /* Set m_mode from conf file */
    string mode = SettingManager::getInstance().getSwapMode();
    if (!setMode(mode)) {
        Logger::error("Invalid Swap Mode : " + mode, getClassName());
        return;
    }

    if (m_mode == SwapMode_NO) {
        Logger::normal("[Success] Mode : " + to_string(m_mode) + \
                       " (0: No, 1: Memory, 2: Full)", getClassName());
        return; /* SwapMode_NO successfully initialized */
    }

    /* Set m_partition from conf file */
    setPartition(SettingManager::getInstance().getSwapPartition());

    /* Set m_size from conf file */
    int size = SettingManager::getInstance().getSwapSize();
    if (!setSize(size)) {
        Logger::error("Invalid Swap Size : " + to_string(size), getClassName());
        return;
    }

    if (!createEFS(m_mode, m_partition, m_size)) {
        Logger::error("Fail to create EFS!", getClassName());
        return;
    }

    if (!createSwap(EFS_MAPPER_PATH)) {
        Logger::error("Fail to create swap space!", getClassName());
        return;
    }

    /* SwapMode_MEMORY or SwapMode_FULL successfully initialized */
    Logger::normal("[Success] Mode : " + to_string(m_mode) + \
                   " (0: No, 1: Memory, 2: Full)" + \
                   ", Partition : " + m_partition + \
                   ", Size : " + to_string(m_size) + " MB", getClassName());
}
