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

#include "SysInfo.h"
#include "util/LinuxProcess.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

map<string, string> SysInfo::memInfo;
vector<pair<string, map<string, long>>> SysInfo::sysView;

long SysInfo::totalPSS = 0;
long SysInfo::cachedPSS = 0;
long SysInfo::fgPSS = 0;
long SysInfo::systemPSS = 0;

bool SysInfo::comparePss(const pair<string, long>& a, const pair<string, long>& b)
{
    return a.second > b.second;
}

long SysInfo::parseSizeToKb(string size)
{
    string value = "", unit = "";
    long ret = 0;

    for (auto &c : size) {
        if (isdigit(static_cast<int>(c)))
            value.push_back(c);
        if (isalpha(static_cast<int>(c))) {
            unit.push_back(toupper(static_cast<int>(c)));
        }
    }

    if (unit == "B" || unit == "BYTE") {
        ret = stol(value) / 1024;
    } else if (unit == "KB" || unit == "K" || unit == "KILOBYTE" || unit == "") {
        ret = stol(value);
    } else if (unit == "MB" || unit == "M" || unit == "MEGABYTE") {
        ret = stol(value) * 1024;
    } else if (unit == "GB" || unit == "G" || unit == "GIGABYTE") {
        ret = stol(value) * 1024 * 1024;
    } else if (unit == "TB" || unit == "T" || unit == "TERABYTE") {
        ret = stol(value) * 1024 * 1024 * 1024;
    } else if (unit == "PB" || unit == "P" || unit == "PETABYTE") {
        ret = stol(value) * 1024 * 1024 * 1024 * 1024;
    } else if (unit == "EB" || unit == "E" || unit == "EXABYTE") {
        ret = stol(value) * 1024 * 1024 * 1024 * 1024 * 1024;
    } else if (unit == "ZB" || unit == "Z" || unit == "ZETTABYTE") {
        ret = stol(value) * 1024 * 1024 * 1024 * 1024 * 1024 * 1024;
    } else {
        Logger::warning("Invalid unit of size", "sysInfo");
        ret = 0;
    }

    return ret;
}

void SysInfo::makeSystemView(JValue& arrSysView)
{
    const unsigned int nItem = sysView.size();
    volatile unsigned int i = 0;

    JValue obj[nItem], arr[nItem];
    boost::format formKey("%-15s: %ld K");
    string key = "", msg = "";

    for (; i < nItem; i++) {
        obj[i] = pbnjson::Object();
        arr[i] = pbnjson::Array();
    }

    i = 0;

    for (auto& m : sysView) {
        string name = m.first;
        for (auto& p : m.second) {
            string subName = p.first;
            long val = p.second;

            formKey %subName %val;
            arr[i].append(formKey.str());
        }

        obj[i].put(name, arr[i]);
        arrSysView.append(obj[i]);

        i++;
    }
}

string SysInfo::getTotalPhyram()
{
    string lsmemTotal = LinuxProcess::getStdoutFromCmd("lsmem | grep 'Total online'");

    /* If no lsmem exists or 'Total online' not found, round up /proc/MemTotal */
    if (lsmemTotal.empty()) {
        long memTotal = stol(memInfo.find("MemTotal")->second);
        long multiple = 1024 * 1024; /* round up to multiple of 1 G */
        memTotal = ((memTotal + multiple - 1) / multiple) * multiple;
        Logger::verbose("Physical RAM size is estimated: " +
                        to_string(memTotal) + " K", "SysInfo");
        return to_string(memTotal);
    }

    lsmemTotal.erase(std::remove(lsmemTotal.begin(), lsmemTotal.end(), ' '), lsmemTotal.end());
    lsmemTotal.erase(0, lsmemTotal.find(":") + 1);

    return lsmemTotal;
}

void SysInfo::makePssViewMsg(JValue& session, const string& sessionId,
                    map<string, long> pidPss[], map<string, string> pidComm[])
{
    JValue sessionArr = pbnjson::Array();
    const unsigned int nPssCtg = static_cast<const unsigned int>(PssCategory::TOTAL_PSS_CTG);

    const string pssTitle[nPssCtg] = {"System", "Foreground", "Cached"};

    for (volatile unsigned int i = 0; i < nPssCtg; i++) {
        if (pidPss[i].size() == 0 || pidComm[i].size() == 0)
            continue;

        JValue obj = pbnjson::Object();
        JValue arr = pbnjson::Array();
        map<string, long> mpp = pidPss[i];
        map<string, string> mpc = pidComm[i];

        /* Make vector from map for sorting */
        vector<pair<string, long>> vps(mpp.begin(), mpp.end());

        sort(vps.begin(), vps.end(), comparePss);

        for (pair<string, long> ppp: vps) {
            string msg = "";
            msg = to_string(ppp.second) + " K: " + mpc[ppp.first] + " (pid " + ppp.first + ")";
            arr.append(msg);
        }

        obj.put(pssTitle[i], arr);
        sessionArr.append(obj);
    }

    session.put(sessionId, sessionArr);
}

void SysInfo::makePssInfo(JValue& allList, JValue& arrPssView)
{
    string appId = "", stat = "", sPid = "", sPss = "", serviceId = "", sessionId = "";
    int pid = 0;
    volatile unsigned int i = 0;
    long pss = 0, pssCached = 0, pssForeground = 0, pssSystem = 0, pssTotal = 0;
    const unsigned int nPssCtg = static_cast<const unsigned int>(PssCategory::TOTAL_PSS_CTG);

    map<string, long> pidPss[nPssCtg];
    map<string, string> pidComm[nPssCtg];

    for (JValue item : allList["sessions"].items()) {
        JValueUtil::getValue(item, "sessionId", sessionId);

        JValue objSession = pbnjson::Object();

        for (i = 0; i < nPssCtg; i++) {
            pidPss[i].clear();
            pidComm[i].clear();
        }

        for (JValue subItem : item["apps"].items()) {
            JValueUtil::getValue(subItem, "appId", appId);
            JValueUtil::getValue(subItem, "pid", pid);
            JValueUtil::getValue(subItem, "status", stat);
            JValueUtil::getValue(subItem, "pss", sPss);

            if (appId.empty())
                continue;

            pss = stoul(sPss);
            sPid = to_string(pid);

            if (stat == "background" || stat == "preload" || stat == "stop" || stat == "close") {
                cachedPSS += pss;

                const unsigned int idxCachedPss =
                                static_cast<const unsigned int>(PssCategory::CACHED_PSS);
                pidComm[idxCachedPss].insert(make_pair(sPid, appId));
                pidPss[idxCachedPss].insert(make_pair(sPid, pss));
            } else if (stat == "foreground" || stat == "launch" || stat == "splash") {
                fgPSS += pss;

                const unsigned int idxFgPss =
                                static_cast<const unsigned int>(PssCategory::FG_PSS);
                pidComm[idxFgPss].insert(make_pair(sPid, appId));
                pidPss[idxFgPss].insert(make_pair(sPid, pss));
            } else if (stat == "pause") {
                systemPSS += pss;

                const unsigned int idxSysPss =
                                static_cast<const unsigned int>(PssCategory::SYS_PSS);
                pidComm[idxSysPss].insert(make_pair(sPid, appId));
                pidPss[idxSysPss].insert(make_pair(sPid, pss));
            }

            totalPSS += pss;
        }

        for (JValue subItem : item["services"].items()) {
            JValueUtil::getValue(subItem, "serviceId", serviceId);
            JValueUtil::getValue(subItem, "pid", sPid);
            JValueUtil::getValue(subItem, "pss", sPss);

            if (serviceId.empty())
                continue;

            vector<string> pidList;
            vector<string> pssList;
            boost::split(pidList, sPid, boost::is_any_of(" "));
            boost::split(pssList, sPss, boost::is_any_of(" "));

            for (i = 0; i < pidList.size(); ++i) {
                pss = stoul(pssList[i]);

                systemPSS += pss;
                totalPSS += pss;

                const unsigned int idxSysPss =
                                static_cast<const unsigned int>(PssCategory::SYS_PSS);
                pidComm[idxSysPss].insert(make_pair(pidList[i], serviceId));
                pidPss[idxSysPss].insert(make_pair(pidList[i], pss));
            }
        }

        makePssViewMsg(objSession, sessionId, pidPss, pidComm);
        arrPssView.append(objSession);
    }
}

void SysInfo::makeMemInfo(long phyramSize)
{
    long mTotalRam = 0, mBsp = 0, mKernel = 0,
                  mUsedRam = 0, mCachedKernel = 0, mFree = 0,
                  mTotalFree = 0, mCustomKernel = 0,
                  mUsedPss = 0, mCachedPss = 0,
                  mSystemPss = 0, mForegroundPss = 0;

    mTotalRam       = stoul(memInfo.find("MemTotal")->second);
    mBsp            = phyramSize - mTotalRam;

    mKernel         = stoul(memInfo.find("Shmem")->second)
                        + stoul(memInfo.find("SUnreclaim")->second)
                        + stoul(memInfo.find("VmallocUsed")->second)
                        + stoul(memInfo.find("PageTables")->second)
                        + stoul(memInfo.find("KernelStack")->second);
    mUsedPss        = totalPSS - cachedPSS;
    mUsedRam        = mUsedPss + mKernel;

    mCachedPss      = cachedPSS;
    mCachedKernel   = stoul(memInfo.find("Buffers")->second)
                        + stoul(memInfo.find("Cached")->second)
                        + stoul(memInfo.find("SReclaimable")->second)
                        - stoul(memInfo.find("Mapped")->second);
    mFree           = stoul(memInfo.find("MemFree")->second);
    mTotalFree      = mCachedPss + mCachedKernel + mFree; // reclaimable + free

    mCustomKernel   = mTotalRam - mUsedRam - mTotalFree;
    if (mCustomKernel < 0)
        mCustomKernel = 0;

    mSystemPss      = systemPSS;
    mForegroundPss  = fgPSS;

    /* Each map should be inserted sequentially */
    map<string, long> mapEsse, mapFg, mapPerf, mapFree;
    mapEsse.insert(make_pair("BSP", mBsp));
    mapEsse.insert(make_pair("Kernel", mKernel));
    mapEsse.insert(make_pair("Custom Kernel", mCustomKernel));
    mapEsse.insert(make_pair("System PSS", mSystemPss));
    mapFg.insert(make_pair("Foreground PSS", mForegroundPss));
    mapPerf.insert(make_pair("Cached PSS", mCachedPss));
    mapPerf.insert(make_pair("Cached Kernel", mCachedKernel));
    mapFree.insert(make_pair("Free", mFree));

    sysView.emplace_back(make_pair("Essential", mapEsse));
    sysView.emplace_back(make_pair("Foreground", mapFg));
    sysView.emplace_back(make_pair("Performance", mapPerf));
    sysView.emplace_back(make_pair("Free", mapFree));
}

bool SysInfo::print(JValue& allList, JValue& message)
{
    long phyramSize = 0;
    string strPhyramSize = "";

    JValue arrSysView = pbnjson::Array(); /* System View */
    JValue arrPssView = pbnjson::Array(); /* PSS View */

    memInfo.clear();
    sysView.clear();

    totalPSS = 0;
    systemPSS = 0;
    fgPSS = 0;
    cachedPSS = 0;

    Proc::getMemInfo(memInfo);
    if (memInfo.empty()) {
        Logger::warning("Cannot get meminfo", "SysInfo");
        message.put("errorText", "Cannot get 'meminfo'");
        return false;
    }

    strPhyramSize = getTotalPhyram();

    if (strPhyramSize.empty()) {
        Logger::warning("Cannot get total physical ram size", "SysInfo");
        message.put("errorText", "Cannot get total pyhsical ram size");
        return false;
    }

    phyramSize = parseSizeToKb(strPhyramSize);

    makePssInfo(allList, arrPssView);
    makeMemInfo(phyramSize);
    makeSystemView(arrSysView);

    message.put("System View", arrSysView);
    message.put("PSS View", arrPssView);

    return true;
}
