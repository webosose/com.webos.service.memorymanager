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
    LunaServiceProvider *p = static_cast<LunaServiceProvider*>(ctxt);
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

    JValueUtil::getValue(requestPayload, "requiredMemory", requiredMemory);

    returnValue = MemoryManager::getInstance()->onRequireMemory(requiredMemory, errorText);

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

    returnValue = JValueUtil::getValue(requestPayload, "type", type);
    if (!returnValue) {
        errorText = "Fail to get type";
        goto out;
    }

    if (type == "killing")
        subscribed = p->m_managerEventKilling.subscribe(request);
    else
        errorText = "Invalid type";

out:
    if (errorText != "")
        responsePayload.put("errorText", errorText);

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
    JValue responsePayload = pbnjson::Array();
    LunaLogger::logRequest(request, requestPayload, mm->getServiceName());

    bool ret = true;

    JValue allList = pbnjson::Object();
    mm->onSysInfo(allList);

    SysInfo::print(allList, responsePayload);

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
}

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

LS::Handle* LunaConnector::getHandle()
{
    return m_handle;
}

bool LunaConnector::connect(const string& serviceName, GMainLoop* loop)
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
    if (m_handle)
        delete m_handle;
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
