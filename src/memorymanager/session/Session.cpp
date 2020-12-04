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

#include "MemoryManager.h"
#include "session/Session.h"
#include "sam/SAM.h"
#include "base/Runtime.h"

#include "util/Logger.h"
#include "util/LinuxProcess.h"
#include "util/JValueUtil.h"
#include "util/Cgroup.h"

const string SessionMonitor::HOST_SESSION_ID = "host";
const string SessionMonitor::HOST_USER_ID = "root";
const string SessionMonitor::HOST_UID = "0";
const string SessionMonitor::m_externalServiceName = "com.webos.service.sessionmanager";

Session::Session(const string& sessionId,
                 const string& userId,
                 const string& uid)
    : m_sessionId(sessionId),
      m_userId(userId),
      m_uid(uid),
      m_sam(nullptr),
      m_runtime(nullptr)
{
    setClassName("Session");

    m_path = Cgroup::generatePath(sessionId == SessionMonitor::HOST_SESSION_ID,
                                  uid);

    m_sam = new SAM(*this);
    m_runtime = new Runtime(*this);
}

Session::~Session()
{
    if (m_sam)
        delete m_sam;

    if (m_runtime)
        delete m_runtime;
}

void Session::print()
{
    char buf[1024];

    snprintf(buf, 1024, "%8.8s %8.8s %5.5s %s",
             m_sessionId.c_str(),
             m_userId.c_str(),
             m_uid.c_str(),
             m_path.c_str());

    Logger::verbose(buf, getClassName());
}

void Session::print(JValue& json)
{
    json.put("sessionId", m_sessionId);
    json.put("userId", m_userId);
    json.put("uid", m_uid);
    json.put("path", m_path);
}

bool SessionMonitor::onGetSessionList(LSHandle *sh, LSMessage *msg, void *ctxt)
{
    SessionMonitor *p = static_cast<SessionMonitor*>(ctxt);
    Message response(msg);
    JValue payload;

    if (response.isHubError())
        return true;

    payload = JDomParser::fromString(response.getPayload());
    LunaLogger::logSubscription("getSessionList", payload, "SessionMonitor");

    JValue sessionList = pbnjson::Array();
    JValueUtil::getValue(payload, "sessionList", sessionList);

    map<string, Session*> localMap;

    /* Move default session (host) to localMap */
    localMap.insert(make_pair(HOST_SESSION_ID, p->m_sessions[HOST_SESSION_ID]));
    p->m_sessions.erase(HOST_SESSION_ID);

    /* Sync new sessions to previous sessions */
    for (JValue session : sessionList.items()) {
        string sessionId = "", userId = "", uid = "";

        if (!JValueUtil::getValue(session, "sessionId", sessionId))
            continue;

        JValueUtil::getValue(session, "userInfo", "userId", userId);
        uid = LinuxProcess::getStdoutFromCmd("id -u " + sessionId);

        Logger::normal("onGetSessionList " + sessionId, p->getClassName());

        auto it = p->m_sessions.find(sessionId);
        if (it == p->m_sessions.end()) {
            /* Create new session */
            Session *s = new Session(sessionId, userId, uid);
            localMap.insert(make_pair(sessionId, s));
        } else {
            /* Move to localMap for later swap */
            p->m_sessions.erase(it->first);
            localMap.insert(make_pair(it->first, it->second));
        }
    }

    /* Remove unused session */
    auto it = p->m_sessions.begin();
    while (it != p->m_sessions.end()) {
        delete it->second;
        it = p->m_sessions.erase(it);
    }

    /* Update session map */
    p->m_sessions.swap(localMap);
    return true;
}

void SessionMonitor::onConnected()
{
    const string uri = "luna://" + m_externalServiceName + "/getSessionList";
    startSubscribe(uri, onGetSessionList, this, "");

    Logger::normal(getSubscribeServiceName() + " is up", getClassName());
}

void SessionMonitor::onDisconnected()
{
    Logger::normal(getSubscribeServiceName() + " is down", getClassName());
}

SessionMonitor::SessionMonitor()
    : LunaSubscriber(SessionMonitor::m_externalServiceName, "")
{
    setClassName("SessionMonitor");

    Session *s = new Session(HOST_SESSION_ID, HOST_USER_ID, HOST_UID);
    m_sessions.insert(make_pair(HOST_SESSION_ID, s));
}

SessionMonitor::~SessionMonitor()
{
    m_sessions.erase(HOST_SESSION_ID);
    delete m_sessions[HOST_SESSION_ID];
}
