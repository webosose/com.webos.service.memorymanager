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

#ifndef SESSION_SESSION_H_
#define SESSION_SESSION_H_

#include "luna2/LunaConnector.h"
#include "interface/IClassName.h"

#include <map>

class SAM;

class Session : public IClassName {

public:
    Session();
    virtual ~Session();

    Session(string sessionId, string userId, string uid);

    const string& getSessionId() const { return m_sessionId; }
    const string& getUserId() const { return m_userId; }
    const string& getUid() const { return m_uid; }

    bool reclaimMemory(bool includeForeground);
    void printApplications(JValue& json);
    int getSessionAppCount();

private:
    const string m_sessionId; /* unique id which external SessionManager creates for each session */
    const string m_userId;    /* user id which external SessionManager creates for each session */
    const string m_uid;       /* Linux uid asssined to each user on the system */

    SAM* m_sam;
};

class SessionMonitor : public IClassName,
                       public LunaSubscriber {
public:
    SessionMonitor();
    virtual ~SessionMonitor();

    std::map<string, Session*> getSessions() { return m_sessions; }

    // LunaSuscriber
    virtual void onDisconnected();
    virtual void onConnected();

private:
    static const string m_externalServiceName;
    static bool onGetSessionList(LSHandle *sh, LSMessage *msg, void *ctxt);

    std::map<string, Session*> m_sessions;
};

#endif /* SESSION_SESSION_H_ */
