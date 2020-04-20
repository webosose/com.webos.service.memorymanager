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

#ifndef LUNA_LUNAMANAGER_H_
#define LUNA_LUNAMANAGER_H_

#include <iostream>
#include <map>
#include <pbnjson.hpp>
#include <luna-service2/lunaservice.hpp>

#include "base/Application.h"
#include "interface/IManager.h"
#include "interface/ISingleton.h"
#include "luna/service/OldHandle.h"
#include "luna/service/NewHandle.h"
#include "setting/SettingManager.h"

using namespace std;
using namespace pbnjson;
using namespace LS;

enum ErrorCode {
    ErrorCode_NoError,
    ErrorCode_UnknownError,
    ErrorCode_WrongJSONFormatError,
    ErrorCode_NoRequiredParametersError,
    ErrorCode_InvalidParametersError,
    ErrorCode_LS2InternalError,
    ErrorCode_UnsupportedAPI
};

class LunaManagerListener {
public:
    LunaManagerListener() {};
    virtual ~LunaManagerListener() {};

    virtual bool onRequireMemory(int requiredMemory, string& errorText) = 0;
    virtual bool onMemoryStatus(JValue& responsePayload) = 0;

};

class LunaManager : public IManager<LunaManagerListener>,
                    public ISingleton<LunaManager> {
friend class ISingleton<LunaManager>;
public:
    virtual ~LunaManager();

    // IManager
    void initialize(GMainLoop* mainloop);

    // Signal
    void signalLevelChanged(string prev, string cur);

    // Posts
    void postMemoryStatus();
    void postManagerKillingEvent(Application& application);

    // APIs
    void getMemoryStatus(Message& request, JValue& requestPayload, JValue& responsePayload);
    void getManagerEvent(Message& request, JValue& requestPayload, JValue& responsePayload);
    void requireMemory(Message& request, JValue& requestPayload, JValue& responsePayload);

    // Internal
    void logRequest(Message& request, JValue& requestPayload, string name);
    void logResponse(Message& request, JValue& responsePayload, string name);
    void logSubscription(string api, JValue& returnPayload);
    void logCall(string& url, JValue& callPayload);
    void logReturn(Message& response, JValue& returnPayload);
    void replyError(JValue& response, enum ErrorCode code);

private:
    static const string toString(enum ErrorCode code);

    bool handleRequired(JValue& requestPayload, JValue& responsePayload, string key, string& value);
    bool handleRequired(JValue& requestPayload, JValue& responsePayload, string key, int& value);
    bool handleRequired(JValue& requestPayload, JValue& responsePayload, string key, bool& value);
    bool handleOptional(JValue& requestPayload, JValue& responsePayload, string key, string& value);
    bool handleOptional(JValue& requestPayload, JValue& responsePayload, string key, int& value);
    bool handleOptional(JValue& requestPayload, JValue& responsePayload, string key, bool& value);

    LunaManager();

    OldHandle m_oldHandle;
    NewHandle m_newHandle;

    SubscriptionPoint m_memoryStatus;

    SubscriptionPoint m_managerEventKilling;
    SubscriptionPoint m_managerEventKillingAll;
    SubscriptionPoint m_managerEventKillingWeb;
    SubscriptionPoint m_managerEventKillingNative;

};

#endif /* LUNA_LUNAMANAGER_H_ */
