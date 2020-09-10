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

#include "util/Logger.h"
#include "util/LinuxProcess.h"
#include "util/JValueUtil.h"

#include <map>

const string SessionMonitor::m_externalServiceName = "com.webos.service.sessionmanager";

int Session::getSessionAppCount()
{
    return m_sam->getAppCount();
}

void Session::printApplications(JValue& json)
{
    return m_sam->print(json);
}

bool Session::reclaimMemory(bool includeForeground)
{
    return m_sam->close(includeForeground);
}

Session::Session(string sessionId, string userId, string uid)
    : m_sessionId(sessionId),
      m_userId(userId),
      m_uid(uid),
      m_sam(nullptr)
{
    setClassName("Session");

    m_sam = new SAM(*this);
}

Session::~Session()
{
    if (m_sam)
        delete m_sam;
}

bool SessionMonitor::onGetSessionList(LSHandle *sh, LSMessage *msg, void *ctxt)
{
    SessionMonitor *p = static_cast<SessionMonitor*>(ctxt);

    LS::Message response(msg);
    JValue payload = JDomParser::fromString(response.getPayload());
    JValue sessionList = pbnjson::Array();

    JValueUtil::getValue(payload, "sessionList", sessionList);

    map<string, Session*> localMap;
    Session *s;

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
            s = new Session(sessionId, userId, uid);
            localMap.insert(make_pair(sessionId, s));
        } else {
            /* Move to localMap for later swap */
            localMap.insert(make_pair(it->first, it->second));
            p->m_sessions.erase(it->first);
        }
    }

    /* Remove unused session */
    for (auto it = p->m_sessions.begin(); it != p->m_sessions.end(); it++) {
            delete it->second;
            p->m_sessions.erase(it->first);
    }

    /* Update session map */
    p->m_sessions.swap(localMap);
    return true;
}

void SessionMonitor::onConnected()
{
    string uri = "luna://" + m_externalServiceName + "/getSessionList";
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
}

SessionMonitor::~SessionMonitor()
{

}
