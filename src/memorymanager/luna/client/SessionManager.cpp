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

#include "SessionManager.h"

bool SessionManager::onGetSessionList(LSHandle *sh, LSMessage *reply, void *ctx)
{
    LS::Message response(reply);
    JValue subscripionPayload = JDomParser::fromString(response.getPayload());
    if (SessionManager::getInstance().m_listener) {
        SessionManager::getInstance().m_listener->onSessionChanged(subscripionPayload);
    }
    return true;
}

SessionManager::SessionManager()
    : AbsClient("com.webos.service.sessionmanager")
{
}

SessionManager::~SessionManager()
{
    m_getSessionList.cancel();
}

bool SessionManager::onStatusChange(bool isConnected)
{
    if (isConnected) {
        JValue requestPayload = pbnjson::Object();
        requestPayload.put("subscribe", true);

        m_getSessionList = m_handle->callMultiReply(
                "luna://com.webos.service.sessionmanager/getSessionList",
                requestPayload.stringify().c_str(),
                onGetSessionList,
                this,
                NULL,
                NULL
        );
    } else {
        m_getSessionList.cancel();
    }
    return true;
}
