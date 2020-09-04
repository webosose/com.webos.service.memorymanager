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


#ifndef LUNA_CLIENT_ABSCLIENT_HPP_
#define LUNA_CLIENT_ABSCLIENT_HPP_

#include <iostream>

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

#include "util/Logger.h"
#include "util/JValueUtil.h"

using namespace std;
using namespace pbnjson;
using namespace LS;

class AbsClient {
public:
    static bool onStatusChange(LSHandle *sh, LSMessage *reply, void *ctx)
    {
        LS::Message response(reply);
        JValue subscriptionPayload = JDomParser::fromString(response.getPayload());
        AbsClient* client = (AbsClient*)ctx;

        bool connected = true;
        if (JValueUtil::getValue(subscriptionPayload, "connected", connected)) {
            client->m_isConnected = connected;

            if (client->m_isConnected) {
                Logger::normal(client->m_serviceName + " is up", "AbsClient");
            } else {
                Logger::normal(client->m_serviceName + " is down", "AbsClient");
            }
            client->onStatusChange(connected);
        }
        return true;
    }

    AbsClient(string serviceName, const string& sessionId = "")
        : m_serviceName(serviceName),
          m_sessionId(sessionId),
          m_isConnected(false),
          m_handle(nullptr),
          m_timeout(5000)
    {

    }

    virtual ~AbsClient()
    {
        m_serverStatus.cancel();
    }

    virtual void initialize(Handle* handle)
    {
        m_serverStatus.cancel();

        JValue requestPayload = pbnjson::Object();
        requestPayload.put("serviceName", m_serviceName);
        if (!m_sessionId.empty())
            requestPayload.put("sessionId", m_sessionId);
        m_serverStatus = handle->callMultiReply(
                "luna://com.webos.service.bus/signal/registerServerStatus",
                requestPayload.stringify().c_str(),
                onStatusChange,
                this
        );
        m_handle = handle;
    }

    string& getServiceName()
    {
        return m_serviceName;
    }

    // This callback is called when target service status is changed
    virtual bool onStatusChange(bool isConnected) = 0;

protected:
    string m_serviceName;
    string m_sessionId;
    bool m_isConnected;
    Handle *m_handle;
    int m_timeout;

private:
    Call m_serverStatus;

};

#endif /* LUNA_CLIENT_ABSCLIENT_HPP_ */
