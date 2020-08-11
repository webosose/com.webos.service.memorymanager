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

#include "LunaManager.h"

#include "client/NotificationManager.h"
#include "client/SAM.h"
#include "client/SessionManager.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

#define NAME    "LunaManager"

const string LunaManager::toString(enum ErrorCode code)
{
    switch(code) {
    case ErrorCode_NoError:
        break;

    case ErrorCode_UnknownError:
        return "Unknown Error";

    case ErrorCode_WrongJSONFormatError:
        return "Wrong Json Format Error";

    case ErrorCode_NoRequiredParametersError:
        return "No Required Parameters Error";

    case ErrorCode_InvalidParametersError:
        return "Invalid Parameters Error";

    case ErrorCode_LS2InternalError:
        return "LS2 Internal Error";

    case ErrorCode_UnsupportedAPI:
        return "Unsupported API";

    default:
        return "Unknown Error";
    }
    return "Unknown Error";
}

LunaManager::LunaManager()
{
}

LunaManager::~LunaManager()
{
}

void LunaManager::initialize(GMainLoop* mainloop)
{
    m_oldHandle.initialize(mainloop);
    m_newHandle.initialize(mainloop);

    m_memoryStatus.setServiceHandle(&m_newHandle);
    m_managerEventKilling.setServiceHandle(&m_newHandle);

    m_managerEventKillingAll.setServiceHandle(&m_oldHandle);
    m_managerEventKillingWeb.setServiceHandle(&m_oldHandle);
    m_managerEventKillingNative.setServiceHandle(&m_oldHandle);

    if (!SettingManager::getInstance().isSessionEnabled()) {
        SAM::subscribe();
    } else {
        SessionManager::getInstance().initialize(&m_newHandle);
        SessionManager::getInstance().setListener(this);
    }
}

void LunaManager::onSessionChanged(JValue& subscriptionPayload)
{
    JValue sessionList = pbnjson::Array();

    // mark sessionId as removed
    for (auto it = m_sessions.begin(); it != m_sessions.end(); ++it) {
        it->second = false;
    }

    JValueUtil::getValue(subscriptionPayload, "sessionList", sessionList);
    for (JValue session : sessionList.items()) {
        string sessionId = "";
        if (!JValueUtil::getValue(session, "sessionId", sessionId)) continue;

        if (m_sessions.find(sessionId) == m_sessions.end()) {
            SAM::subscribe(sessionId);
        }
        m_sessions[sessionId] = true;
    }

    // deleted removed sessionId
    for (auto it = m_sessions.cbegin(); it != m_sessions.cend();) {
      if (it->second == false) {
          SAM::unsubscribe(it->first);
          m_sessions.erase(it++);
      } else {
        ++it;
      }
    }
}

void LunaManager::signalLevelChanged(string prev, string cur)
{
    m_oldHandle.sendThresholdChangedSignal(prev, cur);
    m_newHandle.sendLevelChangedSignal(prev, cur);
}

void LunaManager::postMemoryStatus()
{
    JValue subscriptionPayload = pbnjson::Object();

    m_listener->onMemoryStatus(subscriptionPayload);
    subscriptionPayload.put("returnValue", true);
    subscriptionPayload.put("subscribed", true);
    m_memoryStatus.post(subscriptionPayload.stringify().c_str());
}

void LunaManager::postManagerKillingEvent(Application& application)
{
    JValue subscriptionResponse = pbnjson::Object();
    subscriptionResponse.put("id", application.getAppId());
    subscriptionResponse.put("instanceId", application.getInstanceId());
    subscriptionResponse.put("type", "killing");
    subscriptionResponse.put("returnValue", true);
    subscriptionResponse.put("subscribed", true);

    m_managerEventKilling.post(subscriptionResponse.stringify().c_str());
    m_managerEventKillingAll.post(subscriptionResponse.stringify().c_str());

    if (application.getType() == "native" ||
        application.getType() == "native_builtin" ||
        application.getType() == "native_appshell") {
        m_managerEventKillingNative.post(subscriptionResponse.stringify().c_str());
    } else if (application.getType() == "web") {
        m_managerEventKillingWeb.post(subscriptionResponse.stringify().c_str());
    }
}

void LunaManager::getMemoryStatus(Message& request, JValue& requestPayload, JValue& responsePayload)
{
    if (request.isSubscription()) {
        if (m_memoryStatus.subscribe(request)) {
            responsePayload.put("subscribed", true);
        } else {
            responsePayload.put("subscribed", false);
        }
    }

    m_listener->onMemoryStatus(responsePayload);
    responsePayload.put("returnValue", true);
}

void LunaManager::getManagerEvent(Message& request, JValue& requestPayload, JValue& responsePayload)
{
    string type;
    bool subscribe;
    bool returnValue = true;

    if (!JValueUtil::getValue(requestPayload, "type", type) ||
        !JValueUtil::getValue(requestPayload, "subscribe", subscribe)) {
        replyError(responsePayload, ErrorCode_NoRequiredParametersError);
        returnValue = false;
        goto Done;
    }

    if (!subscribe) {
        replyError(responsePayload, ErrorCode_InvalidParametersError);
        returnValue = false;
        goto Done;
    }

    if (type == "killing") {
        m_managerEventKilling.subscribe(request);
    } else if (type == "killingAll") {
        m_managerEventKillingAll.subscribe(request);
    } else if (type == "killingWeb") {
        m_managerEventKillingWeb.subscribe(request);
    } else if (type == "killingNative") {
        m_managerEventKillingNative.subscribe(request);
    } else {
        replyError(responsePayload, ErrorCode_InvalidParametersError);
        returnValue = false;
        goto Done;
    }

Done:
    responsePayload.put("returnValue", returnValue);
    responsePayload.put("subscribed", subscribe);
}

void LunaManager::requireMemory(Message& request, JValue& requestPayload, JValue& responsePayload)
{
    int requiredMemory;
    if (!JValueUtil::getValue(requestPayload, "requiredMemory", requiredMemory)) {
        replyError(responsePayload, ErrorCode_NoRequiredParametersError);
        return;
    }

    bool relaunch = false;
    JValueUtil::getValue(requestPayload, "relaunch", relaunch);

    bool returnValue = true;
    string errorText = "";
    if (relaunch) {
        goto Done;
    }

    if (requiredMemory <= 0) {
        requiredMemory = SettingManager::getInstance().getDefaultRequiredMemory();
    }

    returnValue = m_listener->onRequireMemory(requiredMemory, errorText);

Done:
    if (!returnValue) {
        responsePayload.put("errorText", errorText);
    }
    responsePayload.put("returnValue", returnValue);
}

void LunaManager::logRequest(Message& request, JValue& requestPayload, string name)
{
    if (SettingManager::getInstance().isVerbose()) {
        Logger::normal("[Request] API(" + string(request.getMethod()) + ") Client(" + string(request.getSenderServiceName())+ ")\n" +
                       requestPayload.stringify("    ").c_str(), name);
    } else {
        Logger::normal("[Request] API(" + string(request.getMethod()) + ") Client(" + string(request.getSenderServiceName())+ ")");
    }
}

void LunaManager::logResponse(Message& request, JValue& responsePayload, string name)
{
    if (SettingManager::getInstance().isVerbose()) {
        Logger::normal("[Response] API(" + string(request.getMethod()) + ") Client(" + string(request.getSenderServiceName())+ ")\n" +
                       responsePayload.stringify("    ").c_str(), name);
    } else {
        Logger::normal("[Response] API(" + string(request.getMethod()) + ") Client(" + string(request.getSenderServiceName())+ ")");
    }
}

void LunaManager::logCall(string& url, JValue& callPayload)
{
    if (SettingManager::getInstance().isVerbose()) {
        Logger::normal("[Call] API(" + url + ")\n" + callPayload.stringify("    ").c_str(), NAME);
    } else {
        Logger::normal("[Call] API(" + url + ")", NAME);
    }
}

void LunaManager::logSubscription(string api, JValue& returnPayload)
{
    if (SettingManager::getInstance().isVerbose()) {
        Logger::normal("[Subscription] API(" + api + ")\n" +
                       returnPayload.stringify("    ").c_str(), NAME);
    } else {
        Logger::normal("[Subscription] API(" + api + ")", NAME);
    }
}

void LunaManager::logReturn(Message& response, JValue& returnPayload)
{
    if (SettingManager::getInstance().isVerbose()) {
        Logger::normal("[Return] Service(" + string(response.getSenderServiceName()) + ")\n" +
                       returnPayload.stringify("    ").c_str(), NAME);
    } else {
        Logger::normal("[Return] Service(" + string(response.getSenderServiceName()) + ")", NAME);
    }
}

void LunaManager::replyError(JValue& responsePayload, enum ErrorCode code)
{
    responsePayload.put("errorCode", code);
    responsePayload.put("errorText", toString(code));
}
