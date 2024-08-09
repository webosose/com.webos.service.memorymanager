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

#include <iostream>
#include <sstream>
#include <string>

#define LOG_NAME "PROC"

void Proc::getMemInfo(map<string, string>& mInfo)
{
    std::ifstream ifs("/proc/meminfo");

    if (!ifs.is_open())
        return;

    std::string line;
    while(std::getline(ifs, line))
    {
        //Logger::debug("getMemInfo: line" + line);
        std::wstring::size_type key_pos = line.find_first_not_of(' ');
        std::wstring::size_type key_end = line.find(":");
        std::wstring::size_type val_pos = line.find_first_not_of(' ', key_end + 1);
        std::wstring::size_type val_end = line.find(' ', val_pos);
        std::string key, val;

        key = line.substr(key_pos, key_end - key_pos);
        val = line.substr(val_pos, val_end - val_pos);

        //Logger::debug("getMemInfo: insert key " + key + ", value " + val);
        mInfo.insert(make_pair(key, val));
    }
    ifs.close();
}

bool Proc::getSmapsRollup(const int pid, map<string, string>& smaps_rollup)
{
    string file = "/proc/" + to_string(pid) + "/smaps_rollup";
    Logger::warning("getSmapsRollup: try open " + file);
    std::ifstream ifs(file);

    if (!ifs.is_open())
        return false;

    std::string line;

    /* In case of smaps_rollup, pass first line */
    std::getline(ifs, line);
    Logger::warning("getSmapsRollup: discard first line, " + line);

    while(std::getline(ifs, line))
    {
        Logger::warning("getSmapsRollup: line" + line);
        std::wstring::size_type key_pos = line.find_first_not_of(' ');
        std::wstring::size_type key_end = line.find(":");
        std::wstring::size_type val_pos = line.find_first_not_of(' ', key_end + 1);
        std::wstring::size_type val_end = line.find(' ', val_pos);
        std::string key, val;

        key = line.substr(key_pos, key_end - key_pos);
        val = line.substr(val_pos, val_end - val_pos);

        Logger::warning("getSmapsRollup: insert key " + key + ", value " + val);
        smaps_rollup.insert(make_pair(key, val));
    }
    ifs.close();

    return true;
}
