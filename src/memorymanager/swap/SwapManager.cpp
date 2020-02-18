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

#include "Environment.h"

const char* SwapManager::PATH_EFS_CORE_MODULE = "/lib/modules/4.14.150/extra/dm-eswap.ko";
const char* SwapManager::PATH_EFS_ADAPTOR_MODULE = "/lib/modules/4.14.150/extra/dm-eswap-ad.ko";
const char* SwapManager::NAME_EFS_CTL = "efsctl";

SwapManager::SwapManager()
    : m_mode(SwapMode_NO),
      m_size(0)
{
    setClassName("SwapManager");
}

SwapManager::~SwapManager()
{
}

void SwapManager::initialize(GMainLoop* mainloop)
{
    string mode = SettingManager::getInstance().getSwapMode();
    string partition = SettingManager::getInstance().getSwapPartition();
    int size = SettingManager::getInstance().getSwapSize();
    string cmd;

    /* Set mode from conf file */
    if (strcmp(mode.c_str(), "NO") == 0) {
        m_mode = SwapMode_NO;
        Logger::normal("Swap Mode : " + mode, getClassName());
        return;
    } else if (strcmp(mode.c_str(), "MEMORY") == 0) {
        m_mode = SwapMode_MEMORY;
    } else if (strcmp(mode.c_str(), "FULL") == 0) {
        m_mode = SwapMode_FULL;
    } else {
        Logger::error("Invalid Swap Mode : " + mode, getClassName());
        return;
    }

    /* Check if EFS kernel modules exist */
    if (access(PATH_EFS_CORE_MODULE, R_OK) != 0 ||
        access(PATH_EFS_ADAPTOR_MODULE, R_OK) != 0) {
        Logger::error(mode + " mode requires Enhanced-Flash-Swap modules", getClassName());
        return;
    }

    /* Get partition info from conf file */
    m_partition = partition;

    /* Validate and set swap size */
    if (size < 0) {
        Logger::error("Invalid Swap Size : " + to_string(size), getClassName());
        return;
    } else {
        m_size = size;
    }

    /* Setup EFS device and swap if mode is MEMORY or FULL */
    Logger::normal("Swap Mode : " + to_string(m_mode) + \
                   "Swap Partition : " + m_partition + \
                   "(0: No, 1: Memory, 2: Full) " + \
                   "Swap Size : " + to_string(m_size), getClassName());

    /* Create EFS */
    cmd = NAME_EFS_CTL;
    cmd += " create -p " + m_partition;
    cmd += " -s " + to_string(m_size);
    if (m_mode == SwapMode_FULL)
        cmd += " -w"; // set shrink_enable
    system(cmd.c_str());

    /* Make swapspace */
    system("mkswap /dev/mapper/eswap");

    /* Swap on */
    system("swapon /dev/mapper/eswap");
}
