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

#ifndef SAM_SAM_H_
#define SAM_SAM_H_

#include <list>

#include "luna2/LunaConnector.h"
#include "session/Session.h"
#include "base/Runtime.h"

#include "interface/IClassName.h"

class SAM : public IClassName,
            public LunaSubscriber {
public:
    explicit SAM(Session &session);
    virtual ~SAM();

    bool close(string appId, string instanceId);

    // LunaSuscriber
    virtual void onDisconnected() override final;
    virtual void onConnected() override final;

private:
    static bool onGetAppLifeEvents(LSHandle *sh, LSMessage *msg, void *ctxt);
    static bool onRunning(LSHandle *sh, LSMessage *msg, void *ctxt);

    static const string m_externalServiceName;
    static const int m_closeTimeOutMs;

    void initAppWaitToRun();
    void initDefaultStatus();

    list<Application> m_appsWaitToRun;
    Session& m_session;
};

#endif /* SAM_SAM_H_ */
