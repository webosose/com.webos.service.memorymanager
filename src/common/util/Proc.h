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

#ifndef UTIL_PROC_H_
#define UTIL_PROC_H_

#include <iostream>
#include <fstream>
#include <map>

using namespace std;

class Proc {
public:
    Proc() {}
    virtual ~Proc() {}

    static void getMemInfo(map<string, string>& mInfo);
    static void getSmapsRollup(const int pid, map<string, string>& smaps_rollup);
};

#endif /* UTIL_PROC_H_ */
