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

#include "luna/LunaManager.h"
#include "util/Logger.h"

bool ApplicationManager::_getAppLifeEvents(LSHandle *sh, LSMessage *reply, void *ctx)
{
    ApplicationManager* sam = (ApplicationManager*)ctx;
    Message response(reply);
    JValue responsePayload = JDomParser::fromString(response.getPayload());

    LunaManager::getInstace().logReturn(response, responsePayload);
    if (response.isHubError()) {
        return false;
    }

    Application application;
    application.fromJson(responsePayload);

    if (application.getAppId().empty()) {
        // SAM returns empty appid at first
        return false;
    }

    auto it = Application::find(sam->m_applications, application.getAppId());
    if (it == sam->m_applications.end()) {
        sam->m_applications.emplace_back();
        sam->m_applications.back().fromApplication(application);
        return true;
    }

    it->fromApplication(application);

    if (application.getApplicationStatus() == ApplicationStatus_Foreground) {
        it->updateTime();
        std::sort(sam->m_applications.begin(), sam->m_applications.end(), Application::compare);
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

    LunaManager::getInstace().logReturn(response, responsePayload);
    if (response.isHubError()) {
        return false;
    }

    for (auto it = sam->m_applications.begin(); it != sam->m_applications.end(); ++it) {
        it->removed();
    }

    Application application;
    for (JValue item : responsePayload["running"].items()) {
        application.fromJson(item);

        auto it = Application::find(sam->m_applications, application.getAppId());
        if (it == sam->m_applications.end()) {
            sam->m_applications.emplace_back();
            sam->m_applications.back().fromApplication(application);
        } else {
            it->fromApplication(application);
            it->notRemoved();
        }
    }

    sam->m_applications.erase(std::remove_if(sam->m_applications.begin(),
                                             sam->m_applications.end(),
                                             Application::isRemoved),
                              sam->m_applications.end());
    if (sam->m_listener) sam->m_listener->onApplicationsChanged();
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
    if (m_applications.size() == 0)
        return false;

    if (!includeForeground &&
        m_applications.back().getApplicationStatus() == ApplicationStatus_Foreground)
        return false;

    if (m_applications.back().isClosing())
        return true;

    m_applications.back().closing();
    LunaManager::getInstace().postManagerKillingEvent(m_applications.back());
    string appId = m_applications.back().getAppId();
    return closeByAppId(appId);
}

string ApplicationManager::getForegroundAppId()
{
    if (m_applications.size() == 0)
        return "";

    if (m_applications.front().getApplicationStatus() == ApplicationStatus_Foreground)
        return m_applications.front().getAppId();
    return "";
}

int ApplicationManager::getRunningAppCount()
{
    return m_applications.size();
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

void ApplicationManager::print()
{
    if (m_applications.size() == 0)
        return;
    if (SettingManager::getInstance().isVerbose()) {
        Logger::verbose("Ordered Application List", m_name);
        for (auto it = m_applications.begin(); it != m_applications.end(); ++it) {
            it->print();
        }
    }
}

void ApplicationManager::print(JValue& json)
{
    JValue array = pbnjson::Array();
    for (auto it = m_applications.begin(); it != m_applications.end(); ++it) {
        JValue item = pbnjson::Object();
        it->print(item);
        array.append(item);
    }
    json.put("applications", array);
}
