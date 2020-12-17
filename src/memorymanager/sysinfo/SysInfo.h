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

enum class PssCategory {
    SYS_PSS,
    FG_PSS,
    CACHED_PSS,
    TOTAL_PSS_CTG,
};

class SysInfo {
public:
    static bool print(JValue& allList, JValue& message);

private:
    SysInfo() = delete;
    virtual ~SysInfo();

    static bool comparePss(const pair<string, long>& a, const pair<string, long>& b);
    static long parseSizeToKb(string size);
    static void makeMemInfo(long phyramSize);
    static void makeSystemView(JValue& arrSysView);
    static string getTotalPhyram();
    static void makePssViewMsg(JValue& session, const string& sessionId,
                        map<string, long> pidPss[], map<string, string> pidComm[]);
    static void makePssInfo(JValue& allList, JValue& arrPssView);

    static map<string, string> memInfo;
    static vector<pair<string, map<string, long>>> sysView;

    static long totalPSS;
    static long cachedPSS;
    static long fgPSS;
    static long systemPSS;
};

#endif /* SYSINFO_SYSINFO_H_ */
