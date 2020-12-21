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

#include "MemoryManager.h"
#include "luna2/LunaConnector.h"

#include "util/Logger.h"
#include "util/JValueUtil.h"
#include "sysinfo/SysInfo.h"

#include <glib.h>

const vector<string> LunaServiceProvider::errorCode{"No Error",         // 0
                                                    "Unknown Error",    // 1
                                                    "Wrong Json Format Error",  // 2
                                                    "No Required Parameters Error", // 3
                                                    "Invalid Parameters Error", // 4
                                                    "LS2 Internal Error",       // 5
                                                    "Unsupported API"};

string LunaSubscriber::getSubscribeServiceName()
{
    return m_subscribeServiceName;
}

bool LunaSubscriber::startSubscribe(const string& uri, LSFilterFunc callback,
                                    void *ctxt, const string& sessionId)
{
    if (m_portIndex >= PORTMAX) {
        Logger::error("Too many methods are subscribed. MAX=" + to_string(PORTMAX), "LunaSubscriber");
        return false;
    }

    LunaConnector* connector = LunaConnector::getInstance();
    LS::Handle *handle = connector->getHandle();
    JValue req = pbnjson::Object();

    req.put("subscribe", true);
#if defined(WEBOS_TARGET_DISTRO_WEBOS_AUTO)
    if (sessionId.empty()) {
        m_subscribePorts[m_portIndex] = handle->callMultiReply(uri.c_str(), req.stringify().c_str(),
                                                 callback, ctxt, nullptr, nullptr);
    } else {
        m_subscribePorts[m_portIndex] = handle->callMultiReply(uri.c_str(), req.stringify().c_str(),
                                                 callback, ctxt, nullptr, sessionId.c_str());
    }
#else
    m_subscribePorts[m_portIndex] = handle->callMultiReply(uri.c_str(), req.stringify().c_str(),
                                             callback, ctxt, nullptr);
#endif

    m_portIndex++;

    return true;
}

LunaSubscriber::LunaSubscriber(const string& serviceName,
                               const string& sessionId)
{
    LunaConnector* connector = LunaConnector::getInstance();
    LS::Handle *handle = connector->getHandle();

    JValue req = pbnjson::Object();
    const string uri = "luna://com.webos.service.bus/signal/registerServerStatus";

    int i = 0;
    for (; i < PORTMAX; ++i) {
        m_subscribePorts[i].cancel();
    }
    m_portIndex = 0;
    m_subscribeServiceName = serviceName;

    req.put("serviceName", serviceName);
    req.put("subscribe", true);

    if (!sessionId.empty())
        req.put("sessionId", sessionId);

#if defined(WEBOS_TARGET_DISTRO_WEBOS_AUTO)
    m_subscribePorts[m_portIndex] = handle->callMultiReply(uri.c_str(), req.stringify().c_str(),
                                            onStatusChange, this, nullptr, nullptr);
#else
    m_subscribePorts[m_portIndex] = handle->callMultiReply(uri.c_str(), req.stringify().c_str(),
                                            onStatusChange, this, nullptr);
#endif
    m_portIndex++;
}

bool LunaSubscriber::onStatusChange(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    LunaSubscriber *p = static_cast<LunaSubscriber*>(ctxt);

    Message response(msg);
    JValue payload = JDomParser::fromString(response.getPayload());

    bool connected = true;;

    if (!JValueUtil::getValue(payload, "connected", connected))
        return true;

    if (connected)
        p->onConnected();
    else
        p->onDisconnected();

    return true;
}

LunaSubscriber::~LunaSubscriber()
{
    int i = 0;
    for (; i < PORTMAX; ++i) {
        m_subscribePorts[i].cancel();
    }
}

LSMethod LunaServiceProvider::methods[] = {
    {"requireMemory", LunaServiceProvider::requireMemory, LUNA_METHOD_FLAGS_NONE},
    {"getMemoryStatus", LunaServiceProvider::getMemoryStatus, LUNA_METHOD_FLAGS_NONE},
    {"getManagerEvent", LunaServiceProvider::getManagerEvent, LUNA_METHOD_FLAGS_NONE},
    {"sysInfo", LunaServiceProvider::sysInfo, LUNA_METHOD_FLAGS_NONE},
    {nullptr, nullptr}
};

LSSignal LunaServiceProvider::signals[] = {
    {"levelChanged", LUNA_SIGNAL_FLAGS_NONE},
    {nullptr}
};

bool LunaServiceProvider::requireMemory(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    MemoryManager* mm = MemoryManager::getInstance();

    Message request(msg);
    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();
    LunaLogger::logRequest(request, requestPayload, mm->getServiceName());

    /* Request handling */
    int requiredMemory = 0;
    bool relaunch = false;
    bool returnValue = true;
    string errorText = "";

    if (!JValueUtil::getValue(requestPayload, "requiredMemory", requiredMemory)) {
        int err = 3;
        responsePayload.put("errorCode", err);
        responsePayload.put("errorText", errorCode[err]);
        returnValue = false;
        goto out;
    }

    returnValue = MemoryManager::getInstance()->onRequireMemory(requiredMemory, errorText);

out:
    if (errorText != "")
        responsePayload.put("errorText", errorText);

    responsePayload.put("returnValue", returnValue);

    LunaLogger::logResponse(request, responsePayload, mm->getServiceName());

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool LunaServiceProvider::getMemoryStatus(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    LunaServiceProvider *p = static_cast<LunaServiceProvider*>(ctxt);
    MemoryManager* mm = MemoryManager::getInstance();

    Message request(msg);
    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();
    LunaLogger::logRequest(request, requestPayload, mm->getServiceName());

    /* Request handling */
    bool subscribed = false;
    bool ret = true;

    if (request.isSubscription())
        subscribed = p->m_memoryStatus.subscribe(request);

    MemoryManager::getInstance()->print(responsePayload);

    responsePayload.put("subscribed", subscribed);
    responsePayload.put("returnValue", ret);

    LunaLogger::logResponse(request, responsePayload, mm->getServiceName());
    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool LunaServiceProvider::getManagerEvent(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    LunaServiceProvider *p = static_cast<LunaServiceProvider*>(ctxt);
    MemoryManager* mm = MemoryManager::getInstance();

    Message request(msg);
    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();
    LunaLogger::logRequest(request, requestPayload, mm->getServiceName());

    /* Request handling */
    string type = "";
    string errorText = "";
    bool subscribed = false;
    bool returnValue = true;
    bool required = true;
    int errCode = 0;

#ifdef SUPPORT_LEGACY_API /* Handle getCloseAppId */
    if (requestPayload.hasKey("appType"))
        requestPayload.put("type", "killing");
#endif

    required &= JValueUtil::getValue(requestPayload, "type", type);
    required &= JValueUtil::getValue(requestPayload, "subscribe", subscribed);

    if (!required) {
        errCode = 3;
        errorText = errorCode[errCode];
        goto out;
    }

    if (!subscribed) {
        errCode = 4;
        errorText = errorCode[errCode];
        goto out;
    }

    if (type == "killing")
        subscribed = p->m_managerEventKilling.subscribe(request);
    else {
        errCode = 4;
        errorText = errorCode[errCode];
    }

out:
    if (errorText != "") {
        responsePayload.put("errorCode", errCode);
        responsePayload.put("errorText", errorText);
        returnValue = false;
    }

    responsePayload.put("subscribed", subscribed);
    responsePayload.put("returnValue", returnValue);

    LunaLogger::logResponse(request, responsePayload, mm->getServiceName());
    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool LunaServiceProvider::sysInfo(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    MemoryManager* mm = MemoryManager::getInstance();

    Message request(msg);
    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();
    LunaLogger::logRequest(request, requestPayload, mm->getServiceName());

    bool ret = true;

    JValue sessionList = pbnjson::Object();
    mm->onSysInfo(sessionList);

    SysInfo::print(sessionList, responsePayload);

    responsePayload.put("returnValue", ret);

    LunaLogger::logResponse(request, responsePayload, mm->getServiceName());
    request.respond(responsePayload.stringify().c_str());
    return ret;
}

void LunaServiceProvider::raiseSignalLevelChanged(const string& prev,
                                                  const string& cur)
{
    LunaConnector* connector = LunaConnector::getInstance();
    LS::Handle *handle = connector->getHandle();

    pbnjson::JValue sig = pbnjson::Object();
    const string uri = "luna://com.webos.service.memorymanager/levelChanged";

    sig.put("previous", prev);
    sig.put("current", cur);

    handle->sendSignal(uri.c_str(), sig.stringify().c_str(), false);

#ifdef SUPPORT_LEGACY_API
    raiseSignalThresholdChanged(prev, cur);
#endif
}

#ifdef SUPPORT_LEGACY_API
LSMethod LunaServiceProvider::oldMethods[] = {
    {"getCloseAppId", LunaServiceProvider::getCloseAppId, LUNA_METHOD_FLAGS_NONE},
    {"getCurrentMemState", LunaServiceProvider::getCurrentMemState, LUNA_METHOD_FLAGS_NONE},
    {nullptr, nullptr}
};

LSSignal LunaServiceProvider::oldSignals[] = {
    {"thresholdChanged", LUNA_SIGNAL_FLAGS_NONE},
    {nullptr}
};

bool LunaServiceProvider::getCloseAppId(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    return getManagerEvent(sh, msg, ctxt);
}

bool LunaServiceProvider::getCurrentMemState(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    return getMemoryStatus(sh, msg, ctxt);
}

void LunaServiceProvider::raiseSignalThresholdChanged(const string& prev,
                                                      const string& cur)
{
    LunaConnector* connector = LunaConnector::getInstance();
    LS::Handle *handle = connector->getHandle();

    pbnjson::JValue sig = pbnjson::Object();
    const string uri = "luna://com.webos.memorymanager/thresholdChanged";

    sig.put("previous", prev);
    sig.put("current", cur);

    MemoryManager* mm = MemoryManager::getInstance();
    auto sessions = mm->getSessionMonitor().getSessions();
    int allAppCount = 0;

    for (auto it = sessions.cbegin(); it != sessions.cend(); ++it)
        allAppCount += it->second->m_runtime->countApp();
    sig.put("remainCount", allAppCount);

    /*
     * TODO : There can be multiple foreground apps due to multiple sessions.
     *        We, however, cannot distinguish whcih foreground app should be
     *        chosen across multiple sessions for now.
     */
    bool found = false;
    for (auto it = sessions.cbegin(); it != sessions.cend(); ++it) {
        string appId = it->second->m_runtime->findFirstForegroundAppId();
        if (!appId.empty()) {
            sig.put("foregroundAppId", appId);
            found = true;
            break;
        }
    }
    if (found == false)
        sig.put("foregroundAppId", "");

    handle->sendSignal(uri.c_str(), sig.stringify().c_str(), false);
}
#endif

void LunaServiceProvider::postMemoryStatus()
{
    JValue payload = pbnjson::Object();

    payload.put("returnValue", true);
    payload.put("subscribed", true);

    MemoryManager::getInstance()->print(payload);
    m_memoryStatus.post(payload.stringify().c_str());
}

void LunaServiceProvider::postManagerEventKilling(const string& appId,
                                                  const string& instanceId)
{
    JValue subscriptionResponse = pbnjson::Object();
    subscriptionResponse.put("id", appId);
    subscriptionResponse.put("instanceId", instanceId);
    subscriptionResponse.put("type", "killing");
    subscriptionResponse.put("returnValue", true);
    subscriptionResponse.put("subscribed", true);

    m_managerEventKilling.post(subscriptionResponse.stringify().c_str());
}

#ifdef SUPPORT_LEGACY_API
LS::Handle* LunaConnector::getOldHandle()
{
    return m_oldHandle;
}

bool LunaConnector::oldConnect(const string& oldServiceName,
                            GMainLoop* loop)
{
    try {
        m_oldHandle = new LS::Handle(oldServiceName.c_str());
        m_oldHandle->attachToLoop(loop);
    } catch(const LS::Error& e) {
        Logger::error("Fail to register to luna-bus (legacy)", getClassName());
        return false;
    }

    return true;
}
#endif

LS::Handle* LunaConnector::getHandle()
{
    return m_handle;
}

bool LunaConnector::connect(const string& serviceName,
                            GMainLoop* loop)
{
    try {
        m_handle = new LS::Handle(serviceName.c_str());
        m_handle->attachToLoop(loop);
    } catch(const LS::Error& e) {
        Logger::error("Fail to register to luna-bus", getClassName());
        return false;
    }

    return true;
}

LunaConnector::~LunaConnector()
{
#ifdef SUPPORT_LEGACY_API
    if (m_oldHandle)
        delete m_oldHandle;
#endif
    if (m_handle)
        delete m_handle;
}

LunaServiceProvider::LunaServiceProvider()
{
    setClassName("LunaServiceProvider");

    LSError error;
    LSErrorInit(&error);

    LunaConnector* connector = LunaConnector::getInstance();

#ifdef SUPPORT_LEGACY_API
    LS::Handle* oldHandle = connector->getOldHandle();
    oldHandle->registerCategory("/", oldMethods, oldSignals, nullptr);
    oldHandle->setCategoryData("/", this);
#endif

    LS::Handle* handle = connector->getHandle();
    handle->registerCategory("/", methods, signals, nullptr);
    handle->setCategoryData("/", this);

    /*
     * TODO : MM v1.0 did not support legacy handle subscription, so we do not
     *        call setSeviceHandle for legacy API on v2.0 until it is required.
     */
    m_memoryStatus.setServiceHandle(handle);
    m_managerEventKilling.setServiceHandle(handle);
}

LunaServiceProvider::~LunaServiceProvider()
{

}

void LunaLogger::logRequest(Message& request, JValue& requestPayload,
                               const string& name)
{
    Logger::normal("[Request] API(" + string(request.getMethod()) +
                   ") Client(" + string(request.getSenderServiceName())+ ")\n" +
                   requestPayload.stringify("    ").c_str(), name);
}

void LunaLogger::logResponse(Message& request, JValue& responsePayload,
                                const string& name)
{
    Logger::normal("[Response] API(" + string(request.getMethod()) +
                   ") Client(" + string(request.getSenderServiceName())+ ")\n" +
                   responsePayload.stringify("    ").c_str(), name);
}

void LunaLogger::logSubscription(const string& api, JValue& returnPayload,
                                    const string& name)
{
    Logger::normal("[Subscription] API(" + api + ")\n" +
                   returnPayload.stringify("    ").c_str(), name);
}

LunaLogger::~LunaLogger()
{

}
