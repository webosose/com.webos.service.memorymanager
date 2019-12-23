// Copyright (c) 2018-2019 LG Electronics, Inc.
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
#include "util/JValueUtil.h"
#include "util/Logger.h"

const string ApplicationManager::NAME = "com.webos.applicationManager";

bool ApplicationManager::_getAppLifeEvents(LSHandle *sh, LSMessage *reply, void *ctx)
{
    ApplicationManager* sam = (ApplicationManager*)ctx;
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

    // 'update' only. Adding operation occurs in 'running' handler
    auto it = sam->m_runningList.find(instanceId, appId);
    if (it != sam->m_runningList.getRunningList().end() && it->getStatus() != event) {
        it->setStatus(event);
        sam->m_runningList.sort();
        if (sam->m_listener) sam->m_listener->onApplicationsChanged();
        sam->print();
    }
    return true;
}

bool ApplicationManager::_running(LSHandle *sh, LSMessage *reply, void *ctx)
{
    ApplicationManager* sam = (ApplicationManager*)ctx;
    Message response(reply);
    JValue responsePayload = JDomParser::fromString(response.getPayload());

    LunaManager::getInstace().logSubscription("running", responsePayload);
    if (response.isHubError()) {
        return false;
    }

    sam->m_runningList.setContext(CONTEXT_NOT_EXIST);
    Application application;
    application.setContext(CONTEXT_EXIST);
    application.setStatus("foreground");
    for (JValue item : responsePayload["running"].items()) {
        string appId = "";
        string instanceId = "";
        string processid = "";
        string appType = "";
        int displayId = 0;

        if (JValueUtil::getValue(item, "id", appId))
            application.setAppId(appId);
        if (JValueUtil::getValue(item, "instanceId", instanceId))
            application.setInstanceId(instanceId);
        if (JValueUtil::getValue(item, "appType", appType))
            application.setType(appType);
        if (JValueUtil::getValue(item, "displayId", displayId))
            application.setDisplayId(displayId);
        if (JValueUtil::getValue(item, "processid", processid) && !processid.empty())
            application.setPid(std::stoi(processid));

        auto it = sam->m_runningList.find(application.getInstanceId(), application.getAppId());
        if (it == sam->m_runningList.getRunningList().end()) {
            sam->m_runningList.push(application);
        } else {
            it->setContext(CONTEXT_EXIST);
            it->setDisplayId(application.getDisplayId());
            it->setPid(application.getPid());
        }
    }
    sam->m_runningList.removeContext(CONTEXT_NOT_EXIST);
    sam->m_runningList.sort();
    if (sam->m_listener) sam->m_listener->onApplicationsChanged();
    sam->print();
    return true;
}

ApplicationManager::ApplicationManager()
    : AbsClient(NAME)
    , m_listener(nullptr)
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

bool ApplicationManager::closeApp(bool includeForeground)
{
    if (m_runningList.isEmpty())
        return false;

    Application& application = m_runningList.back();
    if (application.getStatus() == "close" || application.getStatus() == "stop") {
        Logger::normal(application.getAppId() + "'s status is " + application.getStatus(), m_name);
        return false;
    }
    if (!includeForeground && application.getStatus() == "foreground") {
        Logger::warning("Only 'foreground' apps are exist", m_name);
        return false;
    }
    LunaManager::getInstace().postManagerKillingEvent(application);

    if (application.getStatus() == "foreground") {
        NotificationManager::getInstance().createToast(application.getAppId() + " is closed because of memory issue.");
    }

    if (!closeByAppId(application)) {
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

bool ApplicationManager::closeByAppId(Application& application)
{
    JValue callPayload = pbnjson::Object();
    if (!application.getInstanceId().empty())
        callPayload.put("instanceId", application.getInstanceId());
    if (!application.getAppId().empty())
        callPayload.put("id", application.getAppId());
    callPayload.put("tryToMakeScreenshot", true);

    JValue returnPayload;
    return callSync("closeByAppId", callPayload, returnPayload);
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
