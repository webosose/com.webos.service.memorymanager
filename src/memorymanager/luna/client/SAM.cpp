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

#include "luna/client/SAM.h"

#include "NotificationManager.h"
#include "luna/LunaManager.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

RunningList SAM::s_runningList;

bool SAM::onStatusChange(LSHandle *sh, LSMessage *reply, void *ctx)
{
    LS::Message response(reply);
    JValue subscriptionPayload = JDomParser::fromString(response.getPayload());

    LunaManager::getInstance().logSubscription("registerServerStatus", subscriptionPayload);
    if (response.isHubError()) {
        return false;
    }

    bool connected = false;
    if (JValueUtil::getValue(subscriptionPayload, "connected", connected) && connected) {
        JValue requestPayload = pbnjson::Object();
        requestPayload.put("subscribe", true);
        LSCall(
            LunaManager::getInstance().getHandle().get(),
            "luna://com.webos.service.applicationmanager/getAppLifeEvents",
            requestPayload.stringify().c_str(),
            onGetAppLifeEvents,
            nullptr,
            nullptr,
            nullptr
        );

        LSCall(
            LunaManager::getInstance().getHandle().get(),
            "luna://com.webos.service.applicationmanager/running",
            requestPayload.stringify().c_str(),
            onRunning,
            nullptr,
            nullptr,
            nullptr
        );
    }
    return true;
}

bool SAM::onGetAppLifeEvents(LSHandle *sh, LSMessage *reply, void *ctx)
{
    string sessionId = getSessionId(reply);
    Message response(reply);
    JValue subscriptionPayload = JDomParser::fromString(response.getPayload());

    LunaManager::getInstance().logSubscription("getAppLifeEvents", subscriptionPayload);
    if (response.isHubError()) {
        return true;
    }

    string appId = "";
    string instanceId = "";
    string event = "";

    JValueUtil::getValue(subscriptionPayload, "appId", appId);
    JValueUtil::getValue(subscriptionPayload, "instanceId", instanceId);
    JValueUtil::getValue(subscriptionPayload, "event", event);
    if (appId.empty()) {
        return true;
    }

    // "splash" should be ignored because this is not valid event from MM.
    // If MM returns "false", then "splash" application should be removed.
    // In addition, "splash" status is just virtual status.
    // The application doesn't exist in running list.
    if (event == "splash" || event == "stop") {
        return true;
    }
    auto it = s_runningList.find(instanceId, appId);
    if (it != s_runningList.getRunningList().end() && it->getStatus() == event) {
        // SAME event again ==> nothing to do
        return true;
    }

    if (it != s_runningList.getRunningList().end()) {
        it->setStatus(event);
    } else {
        Application& application = s_runningList.push();
        application.setAppId(appId);
        application.setSessionId(sessionId);
        application.setInstanceId(instanceId);
        application.setStatus(event);
    }
    s_runningList.sort();
    return true;
}

bool SAM::onRunning(LSHandle *sh, LSMessage *reply, void *ctx)
{
    string sessionId = getSessionId(reply);
    Message response(reply);
    JValue subscriptionPayload = JDomParser::fromString(response.getPayload());

    LunaManager::getInstance().logSubscription("running", subscriptionPayload);
    if (response.isHubError()) {
        return true;
    }

    s_runningList.setContext(CONTEXT_NOT_EXIST, sessionId);
    for (JValue item : subscriptionPayload["running"].items()) {
        string appId = "";
        string instanceId = "";
        string processid = "";
        string appType = "";
        int displayId = 0;

        JValueUtil::getValue(item, "id", appId);
        JValueUtil::getValue(item, "instanceId", instanceId);
        JValueUtil::getValue(item, "appType", appType);
        JValueUtil::getValue(item, "displayId", displayId);
        JValueUtil::getValue(item, "processid", processid);

        auto it = s_runningList.find(instanceId, appId);
        if (it == s_runningList.getRunningList().end()) {
            Application& application = s_runningList.push();
            application.setContext(CONTEXT_EXIST);
            application.setAppId(appId);
            application.setSessionId(sessionId);
            application.setInstanceId(instanceId);
            application.setType(appType);
            application.setDisplayId(displayId);
            if (!processid.empty())
                application.setPid(std::stoi(processid));
        } else {
            it->setContext(CONTEXT_EXIST);
            it->setDisplayId(displayId);
            it->setType(appType);
            if (!processid.empty())
                it->setPid(std::stoi(processid));
        }
    }
    s_runningList.removeByContext(CONTEXT_NOT_EXIST);
    s_runningList.sort();
    return true;
}

void SAM::subscribe(const string& sessionId)
{
    JValue requestPayload = pbnjson::Object();
    requestPayload.put("subscribe", true);

    if (sessionId.empty()) {
        requestPayload.put("serviceName", "com.webos.service.applicationmanager");
        LSCall(
            LunaManager::getInstance().getHandle().get(),
            "luna://com.webos.service.bus/signal/registerServerStatus",
            requestPayload.stringify().c_str(),
            onStatusChange,
            nullptr,
            nullptr,
            nullptr
        );
        return;
    }

#if defined(WEBOS_TARGET_DISTRO_WEBOS_AUTO)
    LSCallSession(
        LunaManager::getInstance().getHandle().get(),
        "luna://com.webos.service.applicationmanager/getAppLifeEvents",
        requestPayload.stringify().c_str(),
        sessionId.c_str(),
        onGetAppLifeEvents,
        nullptr,
        nullptr,
        nullptr
    );

    LSCallSession(
        LunaManager::getInstance().getHandle().get(),
        "luna://com.webos.service.applicationmanager/running",
        requestPayload.stringify().c_str(),
        sessionId.c_str(),
        onRunning,
        nullptr,
        nullptr,
        nullptr
    );
#endif
}

void SAM::unsubscribe(const string& sessionId)
{
    s_runningList.removeBySessionId(sessionId);
    s_runningList.sort();
}

bool SAM::close(bool includeForeground, string& errorText)
{
    if (s_runningList.isEmpty() || !s_runningList.back().isCloseable()) {
        errorText = "Failed to reclaim required memory. All apps were closed";
        return false;
    }

    Application& application = s_runningList.back();
    if (!includeForeground && application.getStatus() == "foreground") {
        Logger::warning("Only 'foreground' apps are exist", "SAM");
        return false;
    }
    LunaManager::getInstance().postManagerKillingEvent(application);

    if (application.getStatus() == "foreground") {
        NotificationManager::createToast(
            application.getAppId() + " is closed because of memory issue.",
            application.getSessionId()
        );
    }

    JValue requestPayload = pbnjson::Object();
    if (!application.getInstanceId().empty())
        requestPayload.put("instanceId", application.getInstanceId());
    if (!application.getAppId().empty())
        requestPayload.put("id", application.getAppId());
    requestPayload.put("tryToMakeScreenshot", true);

    LS::Call call;
#if defined(WEBOS_TARGET_DISTRO_WEBOS_AUTO)
    call = LunaManager::getInstance().getHandle().callOneReply(
            "luna://com.webos.service.applicationmanager/close",
            requestPayload.stringify().c_str(),
            nullptr,
            application.getSessionId().c_str()
    );
#else
    call = LunaManager::getInstance().getHandle().callOneReply(
            "luna://com.webos.service.applicationmanager/close",
            requestPayload.stringify().c_str(),
            nullptr
    );
#endif
    Message response = call.get(5000);
    JValue responsePayload = JDomParser::fromString(response.getPayload());

    // TODO Need to add some log message here
    return true;
}

string SAM::getForegroundAppId()
{
    return s_runningList.getForegroundAppId();
}

int SAM::getRunningAppCount()
{
    return s_runningList.getCount();
}

void SAM::toJson(JValue& json)
{
    JValue array = pbnjson::Array();
    for (auto it = s_runningList.getRunningList().begin(); it != s_runningList.getRunningList().end(); ++it) {
        JValue item = pbnjson::Object();
        it->print(item);
        array.append(item);
    }
    json.put("applications", array);
}
