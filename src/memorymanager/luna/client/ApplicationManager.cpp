// Copyright (c) 2018 LG Electronics, Inc.
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
#include "util/Logger.h"

bool ApplicationManager::_getAppLifeEvents(LSHandle *sh, LSMessage *reply, void *ctx)
{
    ApplicationManager* sam = (ApplicationManager*)ctx;
    Message response(reply);
    JValue responsePayload = JDomParser::fromString(response.getPayload());

    LunaManager::getInstace().logSubscription("getAppLifeEvents", responsePayload);
    if (response.isHubError()) {
        return false;
    }

    string appId = responsePayload["appId"].asString();
    if (appId.empty()) {
        // SAM returns empty appid at first
        return true;
    }

    enum ApplicationStatus applicationStatus;
    Application::toEnum(responsePayload["event"].asString(), applicationStatus);

    // 'update' only. Adding operation occurs in 'running' handler
    auto it = sam->m_runningList.find(appId);
    if (it != sam->m_runningList.getRunningList().end() &&
        it->getApplicationStatus() != applicationStatus) {
        it->setApplicationStatus(applicationStatus);
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

    sam->m_runningList.resetPid();
    Application application;
    for (JValue item : responsePayload["running"].items()) {
        enum WindowType windowType;
        enum ApplicationType applicationType;

        Application::toEnum(item["defaultWindowType"].asString(), windowType);
        Application::toEnum(item["appType"].asString(), applicationType);

        application.setAppId(item["id"].asString());
        application.setInstanceId(item["instanceId"].asString());
        if (!item["processId"].asString().empty())
            application.setTid(std::stoi(item["processId"].asString()));
        application.setApplicationType(applicationType);
        application.setWindowType(windowType);

        auto it = sam->m_runningList.find(application.getAppId());
        if (it == sam->m_runningList.getRunningList().end()) {
            // Considering new app as foreground app
            application.setApplicationStatus(ApplicationStatus_Foreground);
            sam->m_runningList.push(application);
        } else {
            it->setTid(application.getTid());
            it->setApplicationType(application.getApplicationType());
            it->setWindowType(application.getWindowType());
        }
    }
    sam->m_runningList.removeZeroPid();
    sam->m_runningList.sort();
    if (sam->m_listener) sam->m_listener->onApplicationsChanged();
    sam->print();
    return true;
}

ApplicationManager::ApplicationManager()
    : AbsClient("com.webos.applicationManager")
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
    if (!includeForeground && application.getApplicationStatus() == ApplicationStatus_Foreground)
        return false;
    LunaManager::getInstace().postManagerKillingEvent(application);

    string appId = application.getAppId();
    enum ApplicationStatus status = application.getApplicationStatus();

    if (status == ApplicationStatus_Foreground) {
        NotificationManager::getInstance().createToast(appId + " is closed because of memory issue.");
    }

    if (!closeByAppId(appId)) {
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

bool ApplicationManager::closeByAppId(string& appId)
{
    JValue callPayload = pbnjson::Object();
    callPayload.put("id", appId);
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
