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

#ifndef LUNA_LUNACONNECTOR_H_
#define LUNA_LUNACONNECTOR_H_

#include "interface/IClassName.h"
#include "interface/ISingleton.h"
#include "base/Application.h"

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

using namespace LS;
using namespace pbnjson;

class LunaSubscriber {
public:
    LunaSubscriber() {}
    virtual ~LunaSubscriber();

    LunaSubscriber(string serviceName, string sessionId);

    bool startSubscribe(string uri, LSFilterFunc callback, void *ctxt, string sessionId);
    string getSubscribeServiceName();

    virtual void onDisconnected() = 0;
    virtual void onConnected() = 0;

private:
    static bool onStatusChange(LSHandle* sh, LSMessage* msg, void* ctxt);

    static const int PORTMAX = 10;
    Call m_subscribePorts[PORTMAX];
    int m_portIndex;
    string m_subscribeServiceName;
};

class LunaServiceProvider : public IClassName {
public:
    LunaServiceProvider();
    virtual ~LunaServiceProvider();

    void raiseSignalLevelChanged(string prev, string cur);
    void postMemoryStatus();
    void postManagerKillingEvent(Application& application);

private:
    static LSMethod methods[];
    static LSMethod oldMethods[];
    static LSSignal signals[];

    static bool requireMemory(LSHandle* sh, LSMessage* msg, void* ctxt);
    static bool getMemoryStatus(LSHandle* sh, LSMessage* msg, void* ctxt);
    static bool getManagerEvent(LSHandle* sh, LSMessage* msg, void* ctxt);
    static bool getCloseAppId(LSHandle* sh, LSMessage* msg, void* ctxt);
    static bool handleKillingEvents(Message& request, JValue& requestPayload,
                                    JValue& respponsePayload, void* ctxt);

    LS::SubscriptionPoint m_memoryStatus;
    LS::SubscriptionPoint m_managerEventKilling;
    LS::SubscriptionPoint m_managerEventKillingAll;
    LS::SubscriptionPoint m_managerEventKillingWeb;
    LS::SubscriptionPoint m_managerEventKillingNative;
};

class LunaConnector : public ISingleton<LunaConnector>,
                      public IClassName {
public:
    LunaConnector() : m_handle(nullptr)
    {
        setClassName("LunaConnector");
    }
    virtual ~LunaConnector();

    bool connect(string serviceName, GMainLoop* loop);
    LS::Handle* getHandle();
    LS::Handle* getOldHandle();

private:
    LS::Handle* m_handle;
    LS::Handle* m_oldHandle;
};

#endif /* LUNA_LUNACONNECTOR_H_ */
