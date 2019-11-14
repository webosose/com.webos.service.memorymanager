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

#include "RunningList.h"

RunningList::RunningList()
{

}

RunningList::~RunningList()
{
}

void RunningList::resetPid()
{
    for (auto it = m_applications.begin(); it != m_applications.end(); ++it) {
        it->setTid(0);
    }
}

void RunningList::removeZeroPid()
{
    auto it = m_applications.begin();
    while (it != m_applications.end()) {
        if (it->getTid() == 0) {
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

    if (m_applications.front().getApplicationStatus() == ApplicationStatus_Foreground)
        return m_applications.front().getAppId();
    return "";
}

bool RunningList::push(Application& application)
{
    m_applications.emplace_back(application);
    return true;
}

Application& RunningList::back()
{
    return m_applications.back();
}

Application& RunningList::front()
{
    return m_applications.front();
}

vector<Application>::iterator RunningList::find(string appId)
{
    vector<Application>::iterator it;
    return it = find_if(m_applications.begin(), m_applications.end(),
            [&] (const Application& application) { return application.getAppId() == appId; } );
}

void RunningList::sort()
{
    std::sort(m_applications.begin(), m_applications.end(), Application::compare);
}

bool RunningList::isEmpty()
{
    return m_applications.empty();
}

bool RunningList::isExist(int pid)
{
    vector<Application>::iterator it;
    it = find_if(m_applications.begin(), m_applications.end(),
                 [&] (const Application& application) { return application.getTid() == pid; } );
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
