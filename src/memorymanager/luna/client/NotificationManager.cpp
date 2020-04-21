// Copyright (c) 2018-2020 LG Electronics, Inc.
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

#include "NotificationManager.h"

#include "luna/LunaManager.h"
#include "util/Logger.h"

bool NotificationManager::onCreateToast(LSHandle *sh, LSMessage *reply, void *ctx)
{
    return true;
}

void NotificationManager::createToast(string message, const string& sessionId)
{
    JValue requestPayload = pbnjson::Object();
    requestPayload.put("sourceId", "com.webos.service.memorymanager");
    requestPayload.put("message", message);

#if defined(WEBOS_TARGET_DISTRO_WEBOS_AUTO)
    LSCallSessionOneReply(
        LunaManager::getInstance().getHandle().get(),
        "luna://com.webos.notification/createToast",
        requestPayload.stringify().c_str(),
        sessionId.c_str(),
        onCreateToast,
        nullptr,
        nullptr,
        nullptr
    );
#else
    LSCallOneReply(
        LunaManager::getInstance().getHandle().get(),
        "luna://com.webos.notification/createToast",
        requestPayload.stringify().c_str(),
        onCreateToast,
        nullptr,
        nullptr,
        nullptr
    );
#endif
}
