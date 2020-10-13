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

#ifndef UTIL_CGROUP_H_
#define UTIL_CGROUP_H_

#include <iostream>
#include <map>
#include <list>

#include "util/Logger.h"

using namespace std;

class Cgroup {
public:
    Cgroup() {}
    virtual ~Cgroup() {}

    static string generatePath(const bool isHost, const string& uid);
    static void iterateDir(map<string, list<int>>& p_comm_pids, string path);

private:
    static const string CGROUP_ROOT;
    static const string CGROUP_PROCS;
};

#endif /* UTIL_CGROUP_H_ */
