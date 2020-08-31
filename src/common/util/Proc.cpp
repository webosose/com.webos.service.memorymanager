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

const string Proc::PATH_READLINK_CMD = "/usr/bin/readlink";

void Proc::getMemoryInfo(long& total, long& available)
{
    string type;
    long value;
    string kb;

    ifstream meminfo("/proc/meminfo");
    while (true) {
        meminfo >> type >> value >> kb;

        if (type == "MemTotal:") {
            total = value / 1024;
        }
        if (type == "MemAvailable:") {
            available = value / 1024;
            break;
        }
    }
    meminfo.close();
}

string Proc::findPidNS(int pid)
{
    /*
     * cmd             : readlink /proc/[pid]/ns/pid
     * expected result : pid:[4026531836]
     */
    string cmd = Proc::PATH_READLINK_CMD + " /proc/" + to_string(pid) + "/ns/pid";
    string result = LinuxProcess::getStdoutFromCmd(cmd);

    /*
     * expected regex_match from "pid:[40266318836]"
     * what[0] : pid:[4026531836]
     * what[1] : 4026531836
     */
    boost::regex expr("pid:\\[([0-9]+)\\]");
    boost::smatch what;
    bool isMatchFound = boost::regex_match(result, what, expr);
    if (isMatchFound) {
        return what[1];
    }

    return "";
}
