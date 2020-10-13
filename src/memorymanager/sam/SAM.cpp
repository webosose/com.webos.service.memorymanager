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

#include "sam/SAM.h"
#include "MemoryManager.h"

#include "util/JValueUtil.h"
#include "util/Logger.h"

#include <glib.h>

const string SAM::m_externalServiceName = "com.webos.service.applicationmanager";
const int SAM::m_closeTimeOutMs = 5000;

bool SAM::close(string appId, string instanceId)
{
    JValue payload = pbnjson::Object();
    payload.put("instanceId", instanceId);
    payload.put("id", appId);
    payload.put("tryToMakeScreenshot", true);

    LS::Handle *handle = LunaConnector::getInstance()->getHandle();
    const string uri = "luna://com.webos.service.applicationmanager/close";

    Call call;
    if (m_session.getSessionId().empty()) {
        call = handle->callOneReply(uri.c_str(), payload.stringify().c_str(),
                (const char *)nullptr, (const char *)nullptr);
    } else {
        call = handle->callOneReply(uri.c_str(), payload.stringify().c_str(),
                (const char *)nullptr, m_session.getSessionId().c_str());
    }

    Message response = call.get(m_closeTimeOutMs);
    if (!response) {
        Logger::error("Error: No response from SAM in 5s", "SAM");
        return false;
    }

    if (response.isHubError()) {
        Logger::error("Error: " + string(response.getPayload()), "SAM");
        return false;
    }

    JValue responsePayload = JDomParser::fromString(response.getPayload());
    bool returnValue = false;

    JValueUtil::getValue(responsePayload, "returnValue", returnValue);
    if (returnValue != true) {
        Logger::error("Error: " + string(response.getPayload()), "SAM");
        return false;
    }

    /* Successful response from SAM */
    return true;
}

bool SAM::onGetAppLifeEvents(LSHandle *sh, LSMessage *msg, void *ctxt)
{
    SAM* p = static_cast<SAM*>(ctxt);
    Message response(msg);
    JValue payload;
    string appId = "", instanceId = "", event = "";

    if (response.isHubError())
        return true;

    payload = JDomParser::fromString(response.getPayload());
    JValueUtil::getValue(payload, "appId", appId);
    JValueUtil::getValue(payload, "instanceId", instanceId);
    JValueUtil::getValue(payload, "event", event);

    if (appId.empty())
        return true;

    /* Find App in Runtime */
    if (p->m_session.m_runtime->updateApp(appId, event))
            return true;

    /* If the app is not in runtime, search apps in list which wait to run */
    auto it = p->m_appsWaitToRun.begin();
    for (; it != p->m_appsWaitToRun.end(); ++it) {
        if (it->getAppId() == appId)
            break;
    }

    if (it == p->m_appsWaitToRun.end()) {
        Application* app = new Application(instanceId, appId, "", event, -1);
        p->m_appsWaitToRun.push_back(*app);
    } else {
        it->setStatus(event);
    }

    return true;
}

bool SAM::onRunning(LSHandle *sh, LSMessage *msg, void *ctxt)
{
    SAM* p = static_cast<SAM*>(ctxt);
    Message response(msg);
    JValue payload;

    if (response.isHubError())
        return true;

    payload = JDomParser::fromString(response.getPayload());

    /* Make application vector with up-to-date runningList */
    for (JValue item : payload["running"].items()) {
        string appId = "", pid = "", webPid = "", appType = "";

        JValueUtil::getValue(item, "id", appId);
        JValueUtil::getValue(item, "processid", pid);
        JValueUtil::getValue(item, "webprocessid", webPid);
        JValueUtil::getValue(item, "appType", appType);

        if (!webPid.empty())
            pid = webPid;

        /* Move complete app instance to Runtime */
        auto it = p->m_appsWaitToRun.begin();
        for (; it != p->m_appsWaitToRun.end(); ++it) {
            if (it->getAppId() == appId)
                break;
        }

        if (it == p->m_appsWaitToRun.end())
            continue;

        it->setPid(stoi(pid));
        it->setType(appType);
        p->m_session.m_runtime->addApp(*it);
        p->m_session.m_runtime->updateMemStat(); //TODO: move to sysInfo
        p->m_appsWaitToRun.erase(it);
    }

    return true;
}

bool SAM::initAppWaitToRun()
{
    JValue payload = pbnjson::Object();
    payload.put("subscribe", true);

    LS::Handle *handle = LunaConnector::getInstance()->getHandle();
    const string uri = "luna://" + m_externalServiceName + "/running";

    Call call;
    if (m_session.getSessionId().empty()) {
        call = handle->callOneReply(uri.c_str(), payload.stringify().c_str(),
                (const char *)nullptr, (const char *)nullptr);
    } else {
        call = handle->callOneReply(uri.c_str(), payload.stringify().c_str(),
                (const char *)nullptr, m_session.getSessionId().c_str());
    }

    Message response = call.get(m_closeTimeOutMs);
    if (!response) {
        Logger::error("Error: No response from SAM in 5s", "SAM");
        return false;
    }

    if (response.isHubError()) {
        Logger::error("Error: " + string(response.getPayload()), "SAM");
        return false;
    }

    JValue responsePayload = JDomParser::fromString(response.getPayload());

    for (JValue item : responsePayload["running"].items()) {
        string instanceId = "", appType = "", appId = "", pid = "", webPid = "";

        JValueUtil::getValue(item, "instanceId", instanceId);
        JValueUtil::getValue(item, "id", appId);
        JValueUtil::getValue(item, "appType", appType);
        JValueUtil::getValue(item, "processid", pid);
        JValueUtil::getValue(item, "webprocessid", webPid);

        if (!webPid.empty())
            pid = webPid;

        Application *app = new Application(instanceId, appId, appType, "", stoi(pid));
        m_appsWaitToRun.push_back(*app);
    }

    return true;
}

void SAM::onConnected()
{
    string uri;

    initAppWaitToRun();

    uri = "luna://" + m_externalServiceName + "/running";
    startSubscribe(uri, onRunning, this, m_session.getSessionId());

    uri = "luna://" + m_externalServiceName + "/getAppLifeEvents";
    startSubscribe(uri, onGetAppLifeEvents, this, m_session.getSessionId());

    Logger::normal(getSubscribeServiceName() + " is up", getClassName());
}

void SAM::onDisconnected()
{
    Logger::normal(getSubscribeServiceName() + " is down", getClassName());
}

SAM::SAM(Session& session)
    : LunaSubscriber(SAM::m_externalServiceName, session.getSessionId()),
      m_session(session)
{
    setClassName("SAM");
}

SAM::~SAM()
{

}
