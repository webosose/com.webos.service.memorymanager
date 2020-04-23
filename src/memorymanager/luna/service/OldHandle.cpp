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

#include "OldHandle.h"

#include "luna/LunaManager.h"
#include "luna/client/SAM.h"
#include "util/JValueUtil.h"
#include "util/Logger.h"

const string OldHandle::NAME_SERVICE = "com.webos.memorymanager";
const string OldHandle::NAME_SIGNAL = "com/webos/memory";

const LSSignal OldHandle::TABLE_SIGNAL[2] = {
    { "thresholdChanged", LUNA_SIGNAL_FLAGS_NONE },
    { nullptr }
};

OldHandle::OldHandle()
    : Handle(registerService(NAME_SERVICE.c_str()))
{
    LS_CATEGORY_BEGIN(OldHandle, "/")
        LS_CATEGORY_METHOD(getCloseAppId)
        LS_CATEGORY_METHOD(getCustomThreshold)
        LS_CATEGORY_METHOD(getCurrentMemState)
        LS_CATEGORY_METHOD(setRequiredMemory)
        LS_CATEGORY_METHOD(getGroupInfo)
        LS_CATEGORY_METHOD(getPolicy)
        LS_CATEGORY_METHOD(getUnitList)
        LS_CATEGORY_METHOD(startMemNotifier)
        LS_CATEGORY_METHOD(sendLowMemPopupTest)
        LS_CATEGORY_METHOD(getEFS)
        LS_CATEGORY_METHOD(setMriHelper)
    LS_CATEGORY_END

    this->registerCategory(NAME_SIGNAL.c_str(), nullptr, TABLE_SIGNAL, nullptr);
}

OldHandle::~OldHandle()
{
}

void OldHandle::initialize(GMainLoop* mainloop)
{
    attachToLoop(mainloop);
}

void OldHandle::sendThresholdChangedSignal(string& prev, string& cur)
{
    static string uri = "luna://" + NAME_SERVICE + "/" + NAME_SIGNAL + "/" + "thresholdChanged";

    pbnjson::JValue signalPayload = pbnjson::Object();
    JValueUtil::putValue(signalPayload, "previous", prev);
    JValueUtil::putValue(signalPayload, "current", cur);
    JValueUtil::putValue(signalPayload, "foregroundAppId", SAM::getForegroundAppId());
    JValueUtil::putValue(signalPayload, "remainCount", SAM::getRunningAppCount());

    this->sendSignal(uri.c_str(), signalPayload.stringify().c_str(), false);
}

bool OldHandle::getCloseAppId(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);

    string appType = "";
    JValueUtil::getValue(requestPayload, "appType", appType);

    if (appType == "web") {
        JValueUtil::putValue(requestPayload, "type", "killingWeb");
    } else if (appType == "native") {
        JValueUtil::putValue(requestPayload, "type", "killingNative");
    } else if (appType == "all") {
        JValueUtil::putValue(requestPayload, "type", "killingAll");
    }
    LunaManager::getInstance().getManagerEvent(request, requestPayload, responsePayload);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool OldHandle::getCustomThreshold(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstance().replyError(responsePayload, ErrorCode_UnsupportedAPI);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool OldHandle::getCurrentMemState(LSMessage &message)
{
    Message request(&message);
    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstance().getMemoryStatus(request, requestPayload, responsePayload);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}


bool OldHandle::setRequiredMemory(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstance().requireMemory(request, requestPayload, responsePayload);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool OldHandle::getGroupInfo(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstance().replyError(responsePayload, ErrorCode_UnsupportedAPI);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool OldHandle::getPolicy(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstance().replyError(responsePayload, ErrorCode_UnsupportedAPI);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool OldHandle::getUnitList(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstance().replyError(responsePayload, ErrorCode_UnsupportedAPI);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool OldHandle::startMemNotifier(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstance().replyError(responsePayload, ErrorCode_UnsupportedAPI);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool OldHandle::sendLowMemPopupTest(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstance().replyError(responsePayload, ErrorCode_UnsupportedAPI);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool OldHandle::getEFS(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstance().replyError(responsePayload, ErrorCode_UnsupportedAPI);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool OldHandle::setMriHelper(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstance().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstance().replyError(responsePayload, ErrorCode_UnsupportedAPI);
    LunaManager::getInstance().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}
