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

#ifndef SYSINFO_SYSINFO_H_
#define SYSINFO_SYSINFO_H_

#include <iostream>
#include <map>
#include <algorithm>

#include "util/JValueUtil.h"
#include "util/Proc.h"
#include "util/Logger.h"

using namespace std;

class SysInfo {
public:
    static bool print(JValue& allList, JValue& message);

private:
    SysInfo() = delete;
    virtual ~SysInfo();

    static bool comparePss(const pair<string, unsigned long>& a, const pair<string, unsigned long>& b);
    static unsigned long parseSizeToKb(string size);
    static void makeMemInfo(unsigned long phyramSize);
    static void makeSystemView(JValue& objSysView);
    static string getTotalPhyram();
    static void makePssViewMsg(JValue& session, const string& sessionId, map<string, unsigned long>& pidPss);
    static void makePss(JValue& allList, JValue& arrPssView);

    static map<string, string> pidComm;
    static map<string, string> pidSession;
    static map<string, string> memInfo;
    static map<string, unsigned long> pssInfo;
    static map<string, unsigned long> sysView;
};

#endif /* SYSINFO_SYSINFO_H_ */
