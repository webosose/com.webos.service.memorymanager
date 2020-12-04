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
map<string, unsigned long> SysInfo::sysView;
map<string, unsigned long> SysInfo::ctgView;

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
        Logger::warning("Unknown unit of size", "sysInfo");
        ret = 0;
    }

    return ret;
}

void SysInfo::makeSystemView(JValue& objSysView)
{
    JValue jarr = pbnjson::Array();
    string msg = "";

    msg = "Total RAM: " + to_string(sysView["totalRam"]) + " K";
    jarr.append(msg);

    msg = "Free  RAM: " + to_string(sysView["freeRam"]) + " ( "
                        + to_string(sysView["cachedPss"]) + " K cached pss + "
                        + to_string(sysView["cachedKernel"]) + " K cached kernel + "
                        + to_string(sysView["free"]) + " K free)";
    jarr.append(msg);

    msg = "Used  RAM: " + to_string(sysView["usedRam"]) + " ( "
                        + to_string(sysView["usedPss"]) + " K used pss + "
                        + to_string(sysView["kernel"]) + " K kernel)";
    jarr.append(msg);

    msg = "Lost  RAM: " + to_string(sysView["lostRam"]) + " K";
    jarr.append(msg);

    objSysView.put("System View", jarr);
}

void SysInfo::makeCategorizedView(JValue& objCtgView)
{
    JValue jobj = pbnjson::Object();
    JValue jarr = pbnjson::Array();
    JValue jarrTot = pbnjson::Array();
    string msg = "";

    msg = string("BSP:            ") + to_string(ctgView["bsp"]) + " K";
    jarr.append(msg);
    msg = string("Kernel:         ") + to_string(ctgView["kernel"]) + " K";
    jarr.append(msg);
    msg = string("Custom Kernel:  ") + to_string(ctgView["customKernel"]) + " K";
    jarr.append(msg);
    msg = string("System PSS:     ") + to_string(ctgView["systemPss"]) + " K";
    jarr.append(msg);
    jobj.put("Essential", jarr);
    jarrTot.append(jobj);

    jobj = pbnjson::Object();
    jarr = pbnjson::Array();
    msg = string("Foreground:     ") + to_string(ctgView["foregroundPss"]) + " K";
    jarr.append(msg);
    jobj.put("Foreground PSS", jarr);
    jarrTot.append(jobj);

    jobj = pbnjson::Object();
    jarr = pbnjson::Array();
    msg = string("Cached PSS:     ") + to_string(ctgView["cachedPss"]) + " K";
    jarr.append(msg);
    msg = string("Cached Kernel:  ") + to_string(ctgView["cachedKernel"]) + " K";
    jarr.append(msg);
    jobj.put("Performance", jarr);
    jarrTot.append(jobj);

    jobj = pbnjson::Object();
    jarr = pbnjson::Array();
    msg = string("Free:           ") + to_string(ctgView["free"]) + " K";
    jarr.append(msg);
    jobj.put("Free", jarr);
    jarrTot.append(jobj);

    objCtgView.put("System View", jarrTot);
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

void SysInfo::makePss(JValue& allList, JValue& objPssView)
{
    string appId = "", stat = "", sPid = "", sPss = "", serviceId = "", sessionId = "", msg = "";
    int pid = 0, i = 0;
    unsigned long pss = 0, pssCached = 0, pssForeground = 0, pssSystem = 0, pssTotal = 0;

    JValue sessionList = pbnjson::Array();
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
        sessionList.append(objSession);
    }

    sysView.insert(make_pair("cachedPss", pssCached));
    sysView.insert(make_pair("usedPss", pssTotal - pssCached));
    ctgView.insert(make_pair("systemPss", pssSystem));
    ctgView.insert(make_pair("foregroundPss", pssForeground));
    ctgView.insert(make_pair("cachedPss", pssCached));

    objPssView.put("PSS View", sessionList);
}

void SysInfo::makeMemInfo(unsigned long phyramSize)
{
    unsigned long svTotalRam = 0, svCachedKernel = 0, svFree = 0,
                  svFreeRam = 0, svKernel = 0, svUsedRam = 0,
                  svCachedPss = 0, svUsedPss = 0, svLostRam = 0,
                  cvBsp = 0, cvKernel = 0, cvCustomKernel = 0,
                  cvCachedKernel = 0, cvFree = 0;

    svCachedPss = sysView["cachedPss"];
    svUsedPss = sysView["usedPss"];

    /* System View */
    svTotalRam      = stoul(memInfo.find("MemTotal")->second);
    svCachedKernel  = stoul(memInfo.find("Buffers")->second)
                      + stoul(memInfo.find("Cached")->second)
                      - stoul(memInfo.find("Mapped")->second)
                      + stoul(memInfo.find("SReclaimable")->second);
    svFree          = stoul(memInfo.find("MemFree")->second);
    svFreeRam       = svCachedPss + svCachedKernel + svFree;
    svKernel        = stoul(memInfo.find("Shmem")->second)
                      + stoul(memInfo.find("SUnreclaim")->second)
                      + stoul(memInfo.find("VmallocUsed")->second)
                      + stoul(memInfo.find("PageTables")->second)
                      + stoul(memInfo.find("KernelStack")->second);
    svUsedRam       = svUsedPss + svKernel;
    svLostRam       = svTotalRam - svUsedRam - svFreeRam;

    /* Categorized View */
    cvBsp           = phyramSize - svTotalRam;
    cvKernel        = svKernel;
    cvCustomKernel  = svLostRam;
    cvCachedKernel  = svCachedKernel;
    cvFree          = svFree;

    sysView.insert(make_pair("totalRam", svTotalRam));
    sysView.insert(make_pair("cachedKernel", svCachedKernel));
    sysView.insert(make_pair("free", svFree));
    sysView.insert(make_pair("freeRam", svFreeRam));
    sysView.insert(make_pair("kernel", svKernel));
    sysView.insert(make_pair("usedRam", svUsedRam));
    sysView.insert(make_pair("lostRam", svLostRam));

    ctgView.insert(make_pair("bsp", cvBsp));
    ctgView.insert(make_pair("kernel", cvKernel));
    ctgView.insert(make_pair("customKernel", cvCustomKernel));
    ctgView.insert(make_pair("cachedKernel", cvCachedKernel));
    ctgView.insert(make_pair("free", cvFree));
}

bool SysInfo::print(JValue& allList, JValue& message)
{
    unsigned long phyramSize = 0;
    string strPhyramSize = "";

    sysView.clear();
    ctgView.clear();

    JValue objSysView = pbnjson::Object(); /* System View */
    JValue objPssView = pbnjson::Object(); /* PSS View */
    JValue objCtgView = pbnjson::Object(); /* Categorized View */
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

    makePss(allList, objPssView);
    makeMemInfo(phyramSize);
    makeSystemView(objSysView);
    makeCategorizedView(objCtgView);

    message.append(objCtgView);
    message.append(objPssView);

    return true;
}
