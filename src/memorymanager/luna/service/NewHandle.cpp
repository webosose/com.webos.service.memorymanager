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

#include "NewHandle.h"

#include "luna/LunaManager.h"
#include "util/Logger.h"

const string NewHandle::NAME_SERVICE = "com.webos.service.memorymanager";
const string NewHandle::NAME_SIGNAL = "com/webos/service/memorymanager";

const LSSignal NewHandle::TABLE_SIGNAL[2] = {
    { "levelChanged", LUNA_SIGNAL_FLAGS_NONE },
    { nullptr }
};

NewHandle::NewHandle()
    : Handle(registerService(NAME_SERVICE.c_str()))
{
    LS_CATEGORY_BEGIN(NewHandle, "/")
        LS_CATEGORY_METHOD(getManagerEvent)
        LS_CATEGORY_METHOD(getMemoryStatus)
        LS_CATEGORY_METHOD(requireMemory)
    LS_CATEGORY_END

    this->registerCategory(NAME_SIGNAL.c_str(), nullptr, TABLE_SIGNAL, nullptr);
}

NewHandle::~NewHandle()
{
}

void NewHandle::initialize(GMainLoop* mainloop)
{
    attachToLoop(mainloop);
}

void NewHandle::sendLevelChangedSignal(string& prev, string& cur)
{
    static string uri = "luna://" + NAME_SERVICE + "/" + NAME_SIGNAL + "/" + "levelChanged";

    pbnjson::JValue signalPayload = pbnjson::Object();
    signalPayload.put("previous", prev);
    signalPayload.put("current", cur);

    this->sendSignal(uri.c_str(), signalPayload.stringify().c_str(), false);
}

bool NewHandle::getManagerEvent(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstace().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstace().getManagerEvent(request, requestPayload, responsePayload);
    LunaManager::getInstace().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool NewHandle::getMemoryStatus(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstace().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstace().getMemoryStatus(request, requestPayload, responsePayload);
    LunaManager::getInstace().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}

bool NewHandle::requireMemory(LSMessage &message)
{
    Message request(&message);

    JValue requestPayload = JDomParser::fromString(request.getPayload());
    JValue responsePayload = pbnjson::Object();

    LunaManager::getInstace().logRequest(request, requestPayload, NAME_SERVICE);
    LunaManager::getInstace().requireMemory(request, requestPayload, responsePayload);
    LunaManager::getInstace().logResponse(request, responsePayload, NAME_SERVICE);

    request.respond(responsePayload.stringify().c_str());
    return true;
}
