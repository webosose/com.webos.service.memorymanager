// Copyright (c) 2020 LG Electronics, Inc.  //
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

#include "Cgroup.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

const string Cgroup::CGROUP_ROOT = "/sys/fs/cgroup/unified"; /* TODO: use memcg v2 */
const string Cgroup::CGROUP_PROCS = "cgroup.procs";

string Cgroup::generatePath(const bool isHost, const string& uid)
{
    string p = CGROUP_ROOT;

    if (true == isHost) {
        p += "/system.slice";
    } else {
        p += "/user.slice";
        p += "/user-" + uid + ".slice";
        p += "/user@" + uid + ".service";
    }

    return p;
}

/* Fill p_comm_pids by iterating <path> recursively */
void Cgroup::iterateDir(map<string, list<int>>& p_comm_pids, string path)
{
    for (auto &&x : boost::filesystem::directory_iterator(path)) {
        if (boost::filesystem::is_directory(x.path())) {
            iterateDir(p_comm_pids, x.path().string());
        }
    }

    string procs = path + "/" + CGROUP_PROCS;
    ifstream ifs(procs.c_str());
    if (ifs.fail()) {
        Logger::error("Fail to open cgroup.procs", "Cgroup");
        return;
    }

    while (true) {
        list<int> pids;
        string pid;
        ifs >> pid;

        if (pid.empty()) {
            break;
        } else {
            pids.push_back(stoi(pid));
        }

        boost::filesystem::path comm_path{path.c_str()};
        string key = comm_path.filename().string();

        if (p_comm_pids.find(key) == p_comm_pids.end()) {
            p_comm_pids.insert(make_pair(key, pids));
        } else {
            p_comm_pids[key].merge(pids);
        }
    }
}
