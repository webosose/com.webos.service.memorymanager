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

#include <glib.h>

string LunaSubscriber::getSubscribeServiceName()
{
    return m_subscribeServiceName;
}

bool LunaSubscriber::startSubscribe(string uri, LSFilterFunc callback, void *ctxt,
                                    string sessionId)
{
    if (m_portIndex >= PORTMAX) {
        Logger::error("Too many methods are subscribed. MAX=" + to_string(PORTMAX), "LunaSubscriber");
        return false;
    }

    LunaConnector* connector = LunaConnector::getInstance();
    LS::Handle *handle = connector->getHandle();
    JValue req = pbnjson::Object();

    req.put("subscribe", true);
    if (sessionId.empty()) {
        m_subscribePorts[m_portIndex] = handle->callMultiReply(uri.c_str(), req.stringify().c_str(),
                                                 callback, ctxt, nullptr, nullptr);
    } else {
        m_subscribePorts[m_portIndex] = handle->callMultiReply(uri.c_str(), req.stringify().c_str(),
                                                 callback, ctxt, nullptr, sessionId.c_str());
    }

    m_portIndex++;

    return true;
}

LunaSubscriber::LunaSubscriber(string serviceName, string sessionId)
{
    LunaConnector* connector = LunaConnector::getInstance();
    LS::Handle *handle = connector->getHandle();

    JValue req = pbnjson::Object();
    const string uri = "luna://com.webos.service.bus/signal/registerServerStatus";

    int i = 0;
    for (; i < PORTMAX; i++) {
        m_subscribePorts[i].cancel();
    }
    m_portIndex = 0;
    m_subscribeServiceName = serviceName;

    req.put("serviceName", serviceName);
    req.put("subscribe", true);

    if (!sessionId.empty())
        req.put("sessionId", sessionId);

    m_subscribePorts[m_portIndex] = handle->callMultiReply(uri.c_str(), req.stringify().c_str(),
                                            onStatusChange, this, nullptr, nullptr);
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
    for (; i < PORTMAX; i++) {
        m_subscribePorts[i].cancel();
    }
}

LSMethod LunaServiceProvider::methods[] = {
    {"requireMemory", LunaServiceProvider::requireMemory, LUNA_METHOD_FLAGS_NONE},
    {"getMemoryStatus", LunaServiceProvider::getMemoryStatus, LUNA_METHOD_FLAGS_NONE},
    {"getManagerEvent", LunaServiceProvider::getManagerEvent, LUNA_METHOD_FLAGS_NONE},
    {nullptr, nullptr}
};

LSMethod LunaServiceProvider::oldMethods[] = {
    {"getCloseAppId", LunaServiceProvider::getCloseAppId, LUNA_METHOD_FLAGS_NONE},
    {nullptr, nullptr}
};

LSSignal LunaServiceProvider::signals[] = {
    {"levelChanged", LUNA_SIGNAL_FLAGS_NONE},
    {nullptr}
};

bool LunaServiceProvider::requireMemory(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    LunaServiceProvider *p = static_cast<LunaServiceProvider*>(ctxt);

    Message request(msg);
    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    /* Request handling */

    int requiredMemory = 0;
    bool relaunch = false;
    bool returnValue = true;
    string errorText = "";

    JValueUtil::getValue(requestPayload, "requiredMemory", requiredMemory);
    JValueUtil::getValue(requestPayload, "relaunch", relaunch);

    if (relaunch)
        goto out;

    returnValue = MemoryManager::getInstance()->onRequireMemory(requiredMemory, errorText);

    /* Request handling */

out:
    if (errorText != "")
        responsePayload.put("errorText", errorText);

    responsePayload.put("returnValue", returnValue);
    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool LunaServiceProvider::getMemoryStatus(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    LunaServiceProvider *p = static_cast<LunaServiceProvider*>(ctxt);

    Message request(msg);
    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    /* Request handling */

    bool subscribed = false;
    bool ret = true;

    if (request.isSubscription())
        subscribed = p->m_memoryStatus.subscribe(request);

    MemoryManager::getInstance()->print(responsePayload);
    /* Request handling */

    responsePayload.put("subscribed", subscribed);
    responsePayload.put("returnValue", ret);
    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool LunaServiceProvider::getManagerEvent(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    LunaServiceProvider *p = static_cast<LunaServiceProvider*>(ctxt);

    Message request(msg);
    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();
    bool ret = true;

    ret = p->handleKillingEvents(request, requestPayload, responsePayload, p);

    request.respond(responsePayload.stringify().c_str());

    return ret;
}

bool LunaServiceProvider::getCloseAppId(LSHandle* sh, LSMessage* msg, void* ctxt)
{
    LunaServiceProvider *p = static_cast<LunaServiceProvider*>(ctxt);

    Message request(msg);
    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();
    bool ret = true;

    if (requestPayload.hasKey("appType")) {
        if (requestPayload["appType"].asString() == "web") {
            requestPayload.put("type", "killingWeb");
        } else if (requestPayload["appType"].asString() == "native") {
            requestPayload.put("type", "killingNative");
        } else if (requestPayload["appType"].asString() == "all") {
            requestPayload.put("type", "killingAll");
        }
    }

    ret = p->handleKillingEvents(request, requestPayload, responsePayload, p);

    request.respond(responsePayload.stringify().c_str());

    return ret;
}

bool LunaServiceProvider::handleKillingEvents(Message& request, JValue& requestPayload,
                                                JValue& responsePayload, void* ctxt)
{
    LunaServiceProvider *p = static_cast<LunaServiceProvider*>(ctxt);

    /* Request handling */
    string type = "";
    string errorText = "";
    bool subscribed = false;
    bool returnValue = true;

    returnValue = JValueUtil::getValue(requestPayload, "type", type);
    if (!returnValue) {
        errorText = "Fail to get type";
        goto out;
    }

    returnValue = request.isSubscription();
    if (!returnValue) {
        errorText = "Invalid arguments";
        goto out;
    }

    if (type == "killing")
        subscribed = p->m_managerEventKilling.subscribe(request);
    else if (type == "killingAll")
        subscribed = p->m_managerEventKillingAll.subscribe(request);
    else if (type == "killingWeb")
        subscribed = p->m_managerEventKillingWeb.subscribe(request);
    else if (type == "killingNative")
        subscribed = p->m_managerEventKillingNative.subscribe(request);
    else
        errorText = "Invalid type";

    /* Request handling */
out:
    if (errorText != "")
        responsePayload.put("errorText", errorText);

    responsePayload.put("subscribed", subscribed);
    responsePayload.put("returnValue", returnValue);
    return true;
}

void LunaServiceProvider::raiseSignalLevelChanged(string prev, string cur)
{
    LunaConnector* connector = LunaConnector::getInstance();
    LS::Handle *handle = connector->getHandle();

    pbnjson::JValue sig = pbnjson::Object();
    const string uri = "luna://com.webos.service.memorymanager/levelChanged";

    sig.put("previous", prev);
    sig.put("current", cur);

    handle->sendSignal(uri.c_str(), sig.stringify().c_str(), false);
}

void LunaServiceProvider::postMemoryStatus()
{
    JValue payload = pbnjson::Object();

    payload.put("returnValue", true);
    payload.put("subscribed", true);

    MemoryManager::getInstance()->print(payload);
    m_memoryStatus.post(payload.stringify().c_str());
}

void LunaServiceProvider::postManagerKillingEvent(Application& application)
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

LunaServiceProvider::LunaServiceProvider()
{
    setClassName("LunaServiceProvider");

    LSError error;
    LSErrorInit(&error);

    LunaConnector* connector = LunaConnector::getInstance();

    LS::Handle* handle = connector->getHandle();

    handle->registerCategory("/", methods, signals, nullptr);
    handle->setCategoryData("/", this);

    m_memoryStatus.setServiceHandle(handle);
    m_managerEventKilling.setServiceHandle(handle);
    m_managerEventKillingAll.setServiceHandle(handle);
    m_managerEventKillingWeb.setServiceHandle(handle);
    m_managerEventKillingNative.setServiceHandle(handle);

    /* Support old handler */
    LS::Handle* oldHandle = connector->getOldHandle();
    oldHandle->registerCategory("/", oldMethods, nullptr, nullptr);
    oldHandle->setCategoryData("/", this);
}

LunaServiceProvider::~LunaServiceProvider()
{

}

LS::Handle* LunaConnector::getHandle()
{
    return m_handle;
}

LS::Handle* LunaConnector::getOldHandle()
{
    return m_oldHandle;
}

bool LunaConnector::connect(string serviceName, GMainLoop* loop)
{
    try {
        m_handle = new LS::Handle(serviceName.c_str());
        m_handle->attachToLoop(loop);

        /* Support old handler */
        m_oldHandle = new LS::Handle("com.webos.memorymanager");
        m_oldHandle->attachToLoop(loop);
    } catch(const LS::Error& e) {
        Logger::error("Fail to register to luna-bus", getClassName());
        return false;
    }

    return true;
}

LunaConnector::~LunaConnector()
{
    if (m_handle)
        delete m_handle;

    if (m_oldHandle)
        delete m_oldHandle;
}