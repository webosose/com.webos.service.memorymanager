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

#include "RunningList.h"

#include "luna/LunaManager.h"
#include "setting/SettingManager.h"
#include "util/Logger.h"

RunningList::RunningList()
{

}

RunningList::~RunningList()
{
}

void RunningList::setContext(int context, const string& sessionId)
{
    for (auto it = m_applications.begin(); it != m_applications.end(); ++it) {
        if(it->getSessionId() == sessionId)
            it->setContext(context);
    }
}

void RunningList::removeByContext(int context)
{
    auto it = m_applications.begin();
    while (it != m_applications.end()) {
        if (it->getContext() == context) {
            it = m_applications.erase(it);
        }
        else
            ++it;
    }
}

void RunningList::removeBySessionId(const string& sessionId)
{
    auto it = m_applications.begin();
    while (it != m_applications.end()) {
        if (it->getSessionId() == sessionId) {
            it = m_applications.erase(it);
        }
        else
            ++it;
    }
}

vector<Application>& RunningList::getRunningList()
{
    return m_applications;
}

int RunningList::getCount()
{
    return m_applications.size();
}

string RunningList::getForegroundAppId()
{
    if (m_applications.empty())
        return "";

    if (m_applications.front().getStatus() == "foreground")
        return m_applications.front().getAppId();
    return "";
}

Application& RunningList::push()
{
    m_applications.emplace_back();
    return m_applications.back();
}

Application& RunningList::back()
{
    return m_applications.back();
}

Application& RunningList::front()
{
    return m_applications.front();
}

vector<Application>::iterator RunningList::find(const string& instanceId, const string& appId)
{
    if (!instanceId.empty())
        return findByInstanceId(instanceId);
    else
        return findByAppId(appId);
}

vector<Application>::iterator RunningList::findByAppId(const string& appId)
{
    vector<Application>::iterator it;
    return it = find_if(m_applications.begin(), m_applications.end(),
            [&] (const Application& application) { return application.getAppId() == appId; } );
}

vector<Application>::iterator RunningList::findByInstanceId(const string& instanceId)
{
    vector<Application>::iterator it;
    return it = find_if(m_applications.begin(), m_applications.end(),
            [&] (const Application& application) { return application.getInstanceId() == instanceId; } );
}

void RunningList::sort()
{
    std::sort(m_applications.begin(), m_applications.end(), Application::compare);
    print();
    LunaManager::getInstance().postMemoryStatus();
}

bool RunningList::isEmpty()
{
    return m_applications.empty();
}

bool RunningList::isExist(int pid)
{
    vector<Application>::iterator it;
    it = find_if(m_applications.begin(), m_applications.end(),
                 [&] (const Application& application) { return application.getPid() == pid; } );
    if (it == m_applications.end()) {
        return false;
    }
    return true;
}

bool RunningList::isExist(string appId)
{
    vector<Application>::iterator it;
    it = find_if(m_applications.begin(), m_applications.end(),
                 [&] (const Application& application) { return application.getAppId() == appId; } );
    if (it == m_applications.end()) {
        return false;
    }
    return true;
}

void RunningList::print()
{
    if (m_applications.size() == 0) {
        Logger::verbose("Application List : Empty", "SAM");
        return;
    }
    if (SettingManager::getInstance().isVerbose()) {
        Logger::verbose("Application List", "SAM");
        for (auto it = m_applications.begin(); it != m_applications.end(); ++it) {
            it->print();
        }
    }
}
