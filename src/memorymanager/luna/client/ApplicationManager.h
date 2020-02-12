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

#ifndef LUNA_CLIENT_APPLICATIONMANAGER_H_
#define LUNA_CLIENT_APPLICATIONMANAGER_H_

#include <iostream>
#include <vector>

#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>

#include "AbsClient.hpp"
#include "base/RunningList.h"
#include "base/IPrintable.h"

using namespace std;
using namespace pbnjson;
using namespace LS;

class ApplicationManagerListener {
public:
    ApplicationManagerListener() {};
    virtual ~ApplicationManagerListener() {};

    virtual void onApplicationsChanged() = 0;

};

class ApplicationManager : public AbsClient, public IPrintable {
public:
    static ApplicationManager& getInstance()
    {
        static ApplicationManager s_instance;
        return s_instance;
    }

    virtual ~ApplicationManager();

    // public
    bool closeApp(bool includeForeground = false);

    string getForegroundAppId();
    int getRunningAppCount();

    virtual void setListener(ApplicationManagerListener* listener)
    {
        m_listener = listener;
    }

    // IPrintable
    virtual void print();
    virtual void print(JValue& json);

private:
    static const string NAME;

    static const int CONTEXT_NOT_EXIST = 0;
    static const int CONTEXT_EXIST = 1;

    static bool _getAppLifeEvents(LSHandle *sh, LSMessage *reply, void *ctx);
    static bool _running(LSHandle *sh, LSMessage *reply, void *ctx);

    ApplicationManager();

    virtual void clear();

    // AbsService
    virtual bool onStatusChange(bool isConnected);

    // APIs
    bool getAppLifeEvents();
    bool running();
    bool close(Application& application);
    bool launch(string& appId);

    RunningList m_runningList;
    Call m_getAppLifeEventsCall;
    Call m_runningCall;

    ApplicationManagerListener* m_listener;
};

#endif /* LUNA_CLIENT_APPLICATIONMANAGER_H_ */
