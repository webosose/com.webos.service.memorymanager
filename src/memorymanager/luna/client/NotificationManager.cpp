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

#include "NotificationManager.h"
#include "util/Logger.h"

const string NotificationManager::NAME = "com.webos.notification";

NotificationManager::NotificationManager()
    : AbsClient(NAME)
    , m_isConnected(false)
{
}

NotificationManager::~NotificationManager()
{

}

bool NotificationManager::onStatusChange(bool isConnected)
{
    m_isConnected = isConnected;
    return true;
}

void NotificationManager::createToast(string message)
{
    if (!m_isConnected) {
        Logger::error("Notification service is not running", NAME);
        return;
    }
    JValue callPayload = pbnjson::Object();
    // TODO: sourceId is duplicated with NewHandle class
    callPayload.put("sourceId", "com.webos.service.memorymanager");
    callPayload.put("message", message);

    JValue returnPayload;
    callSync("createToast", callPayload, returnPayload);
}
