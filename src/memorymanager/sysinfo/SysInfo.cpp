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

map<string, string> SysInfo::pidComm;
map<string, string> SysInfo::pidSession;
map<string, string> SysInfo::memInfo;
map<string, unsigned long> SysInfo::pssInfo;
map<string, unsigned long> SysInfo::sysView;

bool SysInfo::comparePss(const pair<string, unsigned long>& a, const pair<string, unsigned long>& b)
{
    return a.second > b.second;
}

unsigned long SysInfo::parseSizeToKb(string size)
{
    string value = "", unit = "";
    unsigned long ret = 0;

    for (auto &c : size) {
        if (isdigit(c))
            value.push_back(c);
        if (isalpha(c)) {
            unit.push_back(toupper(c));
        }
    }

    if (unit == "B" || unit == "BYTE") {
        ret = stoul(value) / 1024;
    } else if (unit == "KB" || unit == "K" || unit == "KILOBYTE" || unit == "") {
        ret = stoul(value);
    } else if (unit == "MB" || unit == "M" || unit == "MEGABYTE") {
        ret = stoul(value) * 1024;
    } else if (unit == "GB" || unit == "G" || unit == "GIGABYTE") {
        ret = stoul(value) * 1024 * 1024;
    } else if (unit == "TB" || unit == "T" || unit == "TERABYTE") {
        ret = stoul(value) * 1024 * 1024 * 1024;
    } else if (unit == "PB" || unit == "P" || unit == "PETABYTE") {
        ret = stoul(value) * 1024 * 1024 * 1024 * 1024;
    } else if (unit == "EB" || unit == "E" || unit == "EXABYTE") {
        ret = stoul(value) * 1024 * 1024 * 1024 * 1024 * 1024;
    } else if (unit == "ZB" || unit == "Z" || unit == "ZETTABYTE") {
        ret = stoul(value) * 1024 * 1024 * 1024 * 1024 * 1024 * 1024;
    } else {
        Logger::warning("Invalid unit of size", "sysInfo");
        ret = 0;
    }

    return ret;
}

void SysInfo::makeSystemView(JValue& objSysView)
{
    JValue obj = pbnjson::Object();
    string msg = "";

    msg = to_string(sysView["bsp"]) + " K";
    obj.put("BSP          ", msg);
    msg = to_string(sysView["kernel"]) + " K";
    obj.put("Kernel       ", msg);
    msg = to_string(sysView["customKernel"]) + " K";
    obj.put("Custom Kernel", msg);
    msg = to_string(sysView["systemPss"]) + " K";
    obj.put("System PSS   ", msg);
    objSysView.put("Essential", obj);

    obj = pbnjson::Object();
    msg = to_string(sysView["foregroundPss"]) + " K";
    obj.put("Foreground   ", msg);
    objSysView.put("Foreground PSS", obj);

    obj = pbnjson::Object();
    msg = to_string(sysView["cachedPss"]) + " K";
    obj.put("Cached PSS   ", msg);
    msg = to_string(sysView["cachedKernel"]) + " K";
    obj.put("Cached Kernel", msg);
    objSysView.put("Performance", obj);

    obj = pbnjson::Object();
    msg = to_string(sysView["free"]) + " K";
    obj.put("Free         ", msg);
    objSysView.put("Free", obj);

}

string SysInfo::getTotalPhyram()
{
    string lsmemTotal = LinuxProcess::getStdoutFromCmd("lsmem | grep 'Total online'");

    /* If no lsmem exists or 'Total online' not found, round up /proc/MemTotal */
    if (lsmemTotal.empty()) {
        unsigned long memTotal = stoul(memInfo.find("MemTotal")->second);
        unsigned long multiple = 1024 * 1024; /* round up to multiple of 1 G */
        memTotal = ((memTotal + multiple - 1) / multiple) * multiple;
        Logger::verbose("Physical RAM size is estimated: " +
                        to_string(memTotal) + " K", "SysInfo");
        return to_string(memTotal);
    }

    lsmemTotal.erase(std::remove(lsmemTotal.begin(), lsmemTotal.end(), ' '), lsmemTotal.end());
    lsmemTotal.erase(0, lsmemTotal.find(":") + 1);

    return lsmemTotal;
}

void SysInfo::makePssViewMsg(JValue& session, const string& sessionId, map<string, unsigned long>& pidPss)
{
    JValue jarr = pbnjson::Array();

    /* Make vector from map for sorting */
    vector<pair<string, unsigned long>> vPidPss(pidPss.begin(), pidPss.end());

    sort(vPidPss.begin(), vPidPss.end(), comparePss);

    for (pair<string, unsigned long> pairPidPss: vPidPss) {
        string msg = "";
        msg = to_string(pairPidPss.second) + " K: " + pidComm.find(pairPidPss.first)->second
                + " (pid " + pairPidPss.first + ")";
        jarr.append(msg);
    }

    session.put(sessionId, jarr);
}

void SysInfo::makePss(JValue& allList, JValue& arrPssView)
{
    string appId = "", stat = "", sPid = "", sPss = "", serviceId = "", sessionId = "", msg = "";
    int pid = 0, i = 0;
    unsigned long pss = 0, pssCached = 0, pssForeground = 0, pssSystem = 0, pssTotal = 0;

    map<string, unsigned long> pidPss;

    for (JValue item : allList["sessions"].items()) {
        JValueUtil::getValue(item, "sessionId", sessionId);

        JValue objSession = pbnjson::Object();
        pidPss.clear();

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
                pssCached += pss;
            } else if (stat == "foreground" || stat == "launch" || stat == "splash") {
                pssForeground += pss;
            } else if (stat == "pause") {
                pssSystem += pss;
            }

            pssTotal += pss;

            pidComm.insert(make_pair(sPid, appId));
            pidPss.insert(make_pair(sPid, pss));
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

                pssSystem += pss;
                pssTotal += pss;

                pidComm.insert(make_pair(pidList[i], serviceId));
                pidPss.insert(make_pair(pidList[i], pss));
            }
        }

        makePssViewMsg(objSession, sessionId, pidPss);
        arrPssView.append(objSession);
    }

    pssInfo.insert(make_pair("cachedPss", pssCached));
    pssInfo.insert(make_pair("usedPss", pssTotal - pssCached));
    pssInfo.insert(make_pair("systemPss", pssSystem));
    pssInfo.insert(make_pair("foregroundPss", pssForeground));
    pssInfo.insert(make_pair("cachedPss", pssCached));
}

void SysInfo::makeMemInfo(unsigned long phyramSize)
{
    unsigned long mTotalRam = 0, mBsp = 0, mKernel = 0,
                  mUsedPss = 0, mUsedRam = 0, mCachedPss = 0,
                  mCachedKernel = 0, mFree = 0, mTotalFree = 0,
                  mCustomKernel = 0, mSystemPss = 0, mForegroundPss = 0;

    mTotalRam       = stoul(memInfo.find("MemTotal")->second);
    mBsp            = phyramSize - mTotalRam;
    mKernel         = stoul(memInfo.find("Shmem")->second)
                        + stoul(memInfo.find("SUnreclaim")->second)
                        + stoul(memInfo.find("VmallocUsed")->second)
                        + stoul(memInfo.find("PageTables")->second)
                        + stoul(memInfo.find("KernelStack")->second);
    mUsedPss        = pssInfo["usedPss"];
    mUsedRam        = mUsedPss + mKernel;
    mCachedPss      = pssInfo["cachedPss"];
    mCachedKernel   = stoul(memInfo.find("Buffers")->second)
                        + stoul(memInfo.find("Cached")->second)
                        + stoul(memInfo.find("SReclaimable")->second)
                        - stoul(memInfo.find("Mapped")->second);
    mFree           = stoul(memInfo.find("MemFree")->second);
    mTotalFree      = mCachedPss + mCachedKernel + mFree; // reclaimable + free
    mCustomKernel   = mTotalRam - mUsedRam - mTotalFree;
    mSystemPss      = pssInfo["systemPss"];
    mForegroundPss  = pssInfo["foregroundPss"];

    sysView.insert(make_pair("bsp", mBsp));
    sysView.insert(make_pair("kernel", mKernel));
    sysView.insert(make_pair("customKernel", mCustomKernel));
    sysView.insert(make_pair("systemPss", mSystemPss));
    sysView.insert(make_pair("foregroundPss", mForegroundPss));
    sysView.insert(make_pair("cachedPss", mCachedPss));
    sysView.insert(make_pair("cachedKernel", mCachedKernel));
    sysView.insert(make_pair("free", mFree));
}

bool SysInfo::print(JValue& allList, JValue& message)
{
    unsigned long phyramSize = 0;
    string strPhyramSize = "";

    pssInfo.clear();
    sysView.clear();

    JValue objSysView = pbnjson::Object(); /* System View */
    JValue arrPssView = pbnjson::Array(); /* PSS View */
    JValue errMsg = pbnjson::Object();

    Proc::getMemInfo(memInfo);
    if (memInfo.empty()) {
        Logger::warning("Cannot get meminfo", "SysInfo");
        errMsg.put("errorText", "Cannot get 'meminfo'");
        message.append(errMsg);
        return false;
    }

    strPhyramSize = getTotalPhyram();

    if (strPhyramSize.empty()) {
        Logger::warning("Cannot get total physical ram size", "SysInfo");
        errMsg.put("errorText", "Cannot get total pyhsical ram size");
        message.append(errMsg);
        return false;
    }

    phyramSize = parseSizeToKb(strPhyramSize);

    makePss(allList, arrPssView);
    makeMemInfo(phyramSize);
    makeSystemView(objSysView);

    message.put("System View", objSysView);
    message.put("PSS View", arrPssView);

    return true;
}
