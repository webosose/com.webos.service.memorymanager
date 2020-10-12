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

#include "Proc.h"
#include "LinuxProcess.h"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#define LOG_NAME "PROC"

void Proc::getMemInfo(map<string, string>& mInfo)
{
    string key;
    string value;
    string kb;

    ifstream f("/proc/meminfo");

    if (f.fail()) {
        Logger::warning("Fail to open /proc/meminfo", LOG_NAME);
        f.close();
        return;
    }

    while (true) {
        f >> key >> value >> kb;

        if (f.fail())
            break;

        key = key.substr(0, key.find(":"));
        mInfo.insert(make_pair(key, value));
    }

    f.close();
}

void Proc::getSmapsRollup(const int pid, map<string, string>& smaps_rollup)
{
    string key;
    string value;
    string kb;

    ifstream f("/proc/" + to_string(pid) + "/smaps_rollup");

    if (f.fail()) {
        Logger::warning("Fail to open /proc/" + to_string(pid) + "/smaps_rollup", LOG_NAME);
        f.close();
        return;
    }

    /* In case of smaps_rollup, pass first line */
    f >> key >> value >> kb;

    while (true) {
        f >> key >> value >> kb;

        if (f.fail())
            break;

        key = key.substr(0, key.find(":"));
        smaps_rollup.insert(make_pair(key, value));
    }

    f.close();

    return;
}
