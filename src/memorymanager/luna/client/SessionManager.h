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

#ifndef LUNA_CLIENT_SESSIONMANAGER_H_
#define LUNA_CLIENT_SESSIONMANAGER_H_

#include "AbsClient.hpp"

#include <iostream>
#include <pbnjson.hpp>
#include <luna-service2/lunaservice.hpp>

#include "interface/IListener.h"
#include "interface/ISingleton.h"
#include "interface/IClassName.h"

using namespace std;
using namespace pbnjson;
using namespace LS;

class SessionManagerListener {
public:
    SessionManagerListener() {};
    virtual ~SessionManagerListener() {};

    virtual void onSessionChanged(JValue& subscriptionPayload) = 0;

};

class SessionManager : public AbsClient,
                       public IListener<SessionManagerListener>,
                       public ISingleton<SessionManager>,
                       public IClassName {
friend class ISingleton<SessionManager>;
public:
    SessionManager();
    virtual ~SessionManager();

protected:
    virtual bool onStatusChange(bool isConnected) override;

private:
    static bool onGetSessionList(LSHandle *sh, LSMessage *reply, void *ctx);

    Call m_getSessionList;

};

#endif /* LUNA_CLIENT_SESSIONMANAGER_H_ */
