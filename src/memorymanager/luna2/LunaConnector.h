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

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

#define SUPPORT_LEGACY_API /* TODO : will be deprecated */

using namespace LS;
using namespace pbnjson;

class LunaSubscriber {
public:
    explicit LunaSubscriber() = default;
    virtual ~LunaSubscriber();

    LunaSubscriber(const string& serviceName, const string& sessionId);

    bool startSubscribe(const string& uri, LSFilterFunc callback,
                        void *ctxt, const string& sessionId);
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
    explicit LunaServiceProvider();
    virtual ~LunaServiceProvider();

    void raiseSignalLevelChanged(const string& prev, const string& cur);
    void postMemoryStatus();
    void postManagerEventKilling(const string& appId, const string& instanceId);

#ifdef SUPPORT_LEGACY_API
    void raiseSignalThresholdChanged(const string& prev, const string& cur);
#endif

private:
    static LSMethod methods[];
    static LSSignal signals[];
    static const vector<string> errorCode;
    static const string nameService;
    static const string nameSignal;

    static bool requireMemory(LSHandle* sh, LSMessage* msg, void* ctxt);
    static bool getMemoryStatus(LSHandle* sh, LSMessage* msg, void* ctxt);
    static bool getManagerEvent(LSHandle* sh, LSMessage* msg, void* ctxt);
    static bool sysInfo(LSHandle* sh, LSMessage* msg, void* ctxt);

#ifdef SUPPORT_LEGACY_API
    static LSMethod oldMethods[];
    static LSSignal oldSignals[];
    static const string nameOldService;
    static const string nameOldSignal;

    static bool getCloseAppId(LSHandle* sh, LSMessage* msg, void* ctxt);
    static bool getCurrentMemState(LSHandle* sh, LSMessage* msg, void* ctxt);
#endif

    LS::SubscriptionPoint m_memoryStatus;
    LS::SubscriptionPoint m_managerEventKilling;
};

class LunaConnector : public ISingleton<LunaConnector>,
                      public IClassName {
public:
    explicit LunaConnector() : m_handle(nullptr)
    {
#ifdef SUPPORT_LEGACY_API
        m_oldHandle = nullptr;
#endif
        setClassName("LunaConnector");
    }
    virtual ~LunaConnector();

#ifdef SUPPORT_LEGACY_API
    bool oldConnect( const string& serviceName, GMainLoop* loop);
    LS::Handle* getOldHandle();
#endif

    bool connect( const string& serviceName, GMainLoop* loop);
    LS::Handle* getHandle();

private:
#ifdef SUPPORT_LEGACY_API
    LS::Handle* m_oldHandle;
#endif
    LS::Handle* m_handle;
};

class LunaLogger {
public:
    explicit LunaLogger() = default;
    virtual ~LunaLogger();

    static void logRequest(Message& request, JValue& requestPayload,
                           const string& name);
    static void logResponse(Message& request, JValue& responsePayload,
                            const string& name);
    static void logSubscription(const string& api, JValue& returnPayload,
                                const string& name);
};

#endif /* LUNA_LUNACONNECTOR_H_ */
