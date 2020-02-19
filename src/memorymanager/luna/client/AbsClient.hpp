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


#ifndef LUNA_CLIENT_ABSCLIENT_HPP_
#define LUNA_CLIENT_ABSCLIENT_HPP_

#include <iostream>

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

#include "luna/LunaManager.h"
#include "util/Logger.h"

using namespace std;
using namespace pbnjson;
using namespace LS;

class AbsClient {
public:
    AbsClient(string name)
        : m_name(name)
        , m_handle(nullptr)
        , m_timeout(5000)
    {

    }

    virtual ~AbsClient()
    {
        try {
            if (m_serverStatus) {
                m_serverStatus.cancel();
            }
        } catch (const LS::Error& e) {
        }
    }

    virtual void initialize(Handle* handle)
    {
        if (m_serverStatus) {
            m_serverStatus.cancel();
        }
        m_serverStatus = handle->registerServerStatus(m_name.c_str(),
                                                      bind(&AbsClient::onStatusChange, this, std::placeholders::_1));
        m_handle = handle;
    }

    // This callback is called when target service status is changed
    virtual bool onStatusChange(bool isConnected) = 0;

    bool callSync(string key, JValue& callPayload, JValue& returnPayload)
    {
        string url = "luna://" + m_name + "/" + key;

        LunaManager::getInstace().logCall(url, callPayload);
        auto call = m_handle->callOneReply(
            url.c_str(),
            callPayload.stringify().c_str()
        );
        auto reply = call.get(m_timeout);
        if (!reply) {
            Logger::error("No reply during timeout");
            return false;
        }
        if (reply.isHubError()) {
            Logger::error(reply.getPayload());
            return false;
        }
        returnPayload = JDomParser::fromString(reply.getPayload());
        LunaManager::getInstace().logReturn(reply, returnPayload);
        return true;
    }

    bool subscribe(Call& call, string key, JValue& requestPayload, LSFilterFunc callback)
    {
        string url = "luna://" + m_name + "/" + key;

        if (call.isActive()) {
            Logger::warning("The call is already active.", m_name);
            call.cancel();
        }

        try {
            LunaManager::getInstace().logCall(url, requestPayload);
            call = m_handle->callMultiReply(
                url.c_str(),
                requestPayload.stringify().c_str()
            );
            call.continueWith(callback, this);
        }
        catch (const LS::Error &e) {
            Logger::error(string(e.what()), m_name);
            return false;
        }
        return true;
    }

    string& getName()
    {
        return m_name;
    }

protected:
    string m_name;
    Handle *m_handle;
    int m_timeout;

private:
    ServerStatus m_serverStatus;

};

#endif /* LUNA_CLIENT_ABSCLIENT_HPP_ */
