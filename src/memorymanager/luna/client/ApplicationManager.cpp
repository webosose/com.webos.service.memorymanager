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

#include "ApplicationManager.h"

#include "NotificationManager.h"
#include "luna/LunaManager.h"
#include "util/LSMessageUtil.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

const string ApplicationManager::NAME = "com.webos.service.applicationmanager";

bool ApplicationManager::_getAppLifeEvents(LSHandle *sh, LSMessage *reply, void *ctx)
{
    ApplicationManager* sam = (ApplicationManager*)ctx;
    string sessionId = LSMessageUtil::getSessionId(reply);

    Message response(reply);
    JValue responsePayload = JDomParser::fromString(response.getPayload());

    LunaManager::getInstace().logSubscription("getAppLifeEvents", responsePayload);
    if (response.isHubError()) {
        return false;
    }

    string appId = "";
    string instanceId = "";
    string event = "";

    JValueUtil::getValue(responsePayload, "appId", appId);
    JValueUtil::getValue(responsePayload, "instanceId", instanceId);
    JValueUtil::getValue(responsePayload, "event", event);
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
    auto it = sam->m_runningList.find(instanceId, appId);
    if (it != sam->m_runningList.getRunningList().end() && it->getStatus() == event) {
        // SAME event again ==> nothing to do
        return true;
    }

    if (it != sam->m_runningList.getRunningList().end()) {
        it->setStatus(event);
    } else {
        Application& application = sam->m_runningList.push();
        application.setAppId(appId);
        application.setSessionId(sessionId);
        application.setInstanceId(instanceId);
        application.setStatus(event);
    }
    sam->m_runningList.sort();
    sam->print();
    return true;
}

bool ApplicationManager::_running(LSHandle *sh, LSMessage *reply, void *ctx)
{
    ApplicationManager* sam = (ApplicationManager*)ctx;
    string sessionId = LSMessageUtil::LSMessageUtil::getSessionId(reply);
    Message response(reply);
    JValue responsePayload = JDomParser::fromString(response.getPayload());

    LunaManager::getInstace().logSubscription("running", responsePayload);
    if (response.isHubError()) {
        return false;
    }

    sam->m_runningList.setContext(CONTEXT_NOT_EXIST);
    for (JValue item : responsePayload["running"].items()) {
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

        auto it = sam->m_runningList.find(instanceId, appId);
        if (it == sam->m_runningList.getRunningList().end()) {
            Application& application = sam->m_runningList.push();
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
    sam->m_runningList.removeContext(CONTEXT_NOT_EXIST);
    sam->m_runningList.sort();
    sam->print();
    if (sam->m_listener) sam->m_listener->onApplicationsChanged();
    return true;
}

ApplicationManager::ApplicationManager()
    : AbsClient(NAME),
      m_listener(nullptr)
{
}

ApplicationManager::~ApplicationManager()
{
    this->clear();
}

void ApplicationManager::clear()
{
    if (m_getAppLifeEventsCall.isActive()) {
        m_getAppLifeEventsCall.cancel();
    }
    if (m_runningCall.isActive()) {
        m_runningCall.cancel();
    }
}

bool ApplicationManager::closeApp(bool includeForeground, string& errorText)
{
    if (m_runningList.isEmpty() || !m_runningList.back().isCloseable()) {
        errorText = "Failed to reclaim required memory. All apps were closed";
        return false;
    }

    Application& application = m_runningList.back();
    if (!includeForeground && application.getStatus() == "foreground") {
        Logger::warning("Only 'foreground' apps are exist", m_name);
        return false;
    }
    LunaManager::getInstace().postManagerKillingEvent(application);

    if (application.getStatus() == "foreground") {
        NotificationManager::getInstance().createToast(application.getAppId() + " is closed because of memory issue.");
    }

    if (!close(application)) {
        Logger::warning("Failed to call closeByAppId", m_name);
        return false;
    }
    return true;
}

string ApplicationManager::getForegroundAppId()
{
    return m_runningList.getForegroundAppId();
}

int ApplicationManager::getRunningAppCount()
{
    return m_runningList.getCount();
}

bool ApplicationManager::onStatusChange(bool isConnected)
{
    if (isConnected) {
        Logger::normal("Connected", m_name);
        running();
        getAppLifeEvents();
    } else {
        Logger::normal("Disconnected", m_name);
        clear();
    }
    return true;
}

bool ApplicationManager::getAppLifeEvents()
{
    JValue callPayload = pbnjson::Object();
    callPayload.put("subscribe", true);

    return subscribe(m_getAppLifeEventsCall, "getAppLifeEvents", callPayload, _getAppLifeEvents);
}

bool ApplicationManager::running()
{
    JValue callPayload = pbnjson::Object();
    callPayload.put("subscribe", true);

    return subscribe(m_runningCall, "running", callPayload, _running);
}

bool ApplicationManager::close(Application& application)
{
    JValue callPayload = pbnjson::Object();
    if (!application.getInstanceId().empty())
        callPayload.put("instanceId", application.getInstanceId());
    if (!application.getAppId().empty())
        callPayload.put("id", application.getAppId());
    callPayload.put("tryToMakeScreenshot", true);

    JValue returnPayload;
    return callSync("close", callPayload, returnPayload);
}

bool ApplicationManager::launch(string& appId)
{
    JValue callPayload = pbnjson::Object();
    callPayload.put("id", appId);

    JValue returnPayload;
    return callSync("launch", callPayload, returnPayload);
}

void ApplicationManager::print()
{
    if (m_runningList.getRunningList().size() == 0) {
        Logger::verbose("Application List : Empty", m_name);
        return;
    }
    if (SettingManager::getInstance().isVerbose()) {
        Logger::verbose("Application List", m_name);
        for (auto it = m_runningList.getRunningList().begin(); it != m_runningList.getRunningList().end(); ++it) {
            it->print();
        }
    }
}

void ApplicationManager::print(JValue& json)
{
    JValue array = pbnjson::Array();
    for (auto it = m_runningList.getRunningList().begin(); it != m_runningList.getRunningList().end(); ++it) {
        JValue item = pbnjson::Object();
        it->print(item);
        array.append(item);
    }
    json.put("applications", array);
}
