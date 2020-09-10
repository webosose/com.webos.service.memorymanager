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
#include "session/Session.h"
#include "MemoryManager.h"

#include "util/JValueUtil.h"
#include "util/Logger.h"

#include <glib.h>

const string SAM::m_externalServiceName = "com.webos.service.applicationmanager";
const int SAM::m_closeTimeOutMs = 5000;

bool SAM::onGetAppLifeEvents(LSHandle *sh, LSMessage *msg, void *ctxt)
{
    SAM* p = static_cast<SAM*>(ctxt);
    Message response(msg);
    JValue payload;
    string appId = "", instanceId = "", event = "", appType = "";
    int displayId = 0;

    if (response.isHubError())
        return true;

    payload = JDomParser::fromString(response.getPayload());
    JValueUtil::getValue(payload, "appId", appId);
    JValueUtil::getValue(payload, "instanceId", instanceId);
    JValueUtil::getValue(payload, "appType", appType);
    JValueUtil::getValue(payload, "event", event);
    JValueUtil::getValue(payload, "displayId", displayId);

    if (appId.empty())
        return true;

    auto it = p->m_runningList.find(instanceId, appId);

    if (it != p->m_runningList.getRunningList().end()) {
        if (it->getStatus() == event)
            return true;

        it->setStatus(event);
    } else {
        Application& localApp = p->m_runningList.push();
        localApp.setProperty(instanceId, appId, appType, 0, displayId);
        localApp.setStatus(event);
    }

    p->m_runningList.sort();

    MemoryManager *mm = MemoryManager::getInstance();
    mm->renewMemoryStatus();

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

    vector<Application> localVector;

    /* Make application vector with up-to-date runningList */
    for (JValue item : payload["running"].items()) {
        string appId = "", instanceId = "", pid = "", appType = "", webPid = "", event = "";
        int displayId = 0;

        JValueUtil::getValue(item, "instanceId", instanceId);
        JValueUtil::getValue(item, "id", appId);
        JValueUtil::getValue(item, "appType", appType);
        JValueUtil::getValue(item, "processid", pid);
        JValueUtil::getValue(item, "displayId", displayId);
        JValueUtil::getValue(item, "webprocessid", webPid);

        if (!webPid.empty())
            pid = webPid;

        localVector.emplace_back();
        Application& localApp = localVector.back();

        auto it = p->m_runningList.find(instanceId, appId);

        if (it != p->m_runningList.getRunningList().end()) {
            event = it->getStatus();
            localApp.setStatus(event);
        }

        localApp.setProperty(instanceId, appId, appType, std::stoi(pid), displayId);
    }

    p->m_runningList.getRunningList().swap(localVector);
    p->m_runningList.sort();

    MemoryManager *mm = MemoryManager::getInstance();
    mm->renewMemoryStatus();

    localVector.clear();
    return true;
}

bool SAM::close(bool includeForeground)
{
    if (m_runningList.isEmpty())
        return false;

    Application& selected = m_runningList.back();

    if (!selected.isCloseable()) {
        return false;
    }

    MemoryManager *mm = MemoryManager::getInstance();
    mm->killApplication(selected);

    if (selected.getStatus() == "foreground" && !includeForeground) {
        Logger::warning("Only 'foreground' apps are exist", getClassName());
        return false;
    }

#if 0
    /* TODO : add after notification manager refactoring */
    if (application.getStatus() == "foreground") {
        NotificationManager::createToast(
            application.getAppId() + " is closed because of memory issue.",
            application.getSessionId()
        );
    }
#endif

    JValue payload = pbnjson::Object();
    payload.put("instanceId", selected.getInstanceId());
    payload.put("id", selected.getAppId());
    payload.put("tryToMakeScreenshot", true);

    LunaConnector* connector = LunaConnector::getInstance();
    LS::Handle *handle = connector->getHandle();
    const string uri = "luna://com.webos.service.applicationmanager/close";

    try {
        Call call;
        if (m_session.getSessionId().empty()) {
            call = handle->callOneReply(uri.c_str(), payload.stringify().c_str(),
                                        (const char *)NULL, (const char *)NULL);
        } else {
            call = handle->callOneReply(uri.c_str(), payload.stringify().c_str(),
                                        (const char *)NULL, m_session.getSessionId().c_str());
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
    } catch(const LS::Error& lse) {
        Logger::error("Exception: " + string(lse.what()), "SAM");
        return false;
    } catch(const std::exception& e) {
        Logger::error("Exception: " + string(e.what()), "SAM");
        return false;
    }
}

int SAM::getAppCount()
{
    return m_runningList.getCount();
}

void SAM::print(JValue& json)
{
    for (auto it = m_runningList.getRunningList().begin();
              it != m_runningList.getRunningList().end(); ++it) {
        JValue item = pbnjson::Object();
        it->print(item);
        json.append(item);
    }
}

void SAM::onConnected()
{
    string uri;

    uri = "luna://" + m_externalServiceName + "/getAppLifeEvents";
    startSubscribe(uri, onGetAppLifeEvents, this, m_session.getSessionId());

    uri = "luna://" + m_externalServiceName + "/running";
    startSubscribe(uri, onRunning, this, m_session.getSessionId());

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
