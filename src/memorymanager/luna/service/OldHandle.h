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

#ifndef LUNA_SERVICE_OLDHANDLE_H_
#define LUNA_SERVICE_OLDHANDLE_H_

#include <iostream>
#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>
#include <glib.h>

using namespace std;
using namespace LS;
using namespace pbnjson;

class OldHandle : public Handle {
public:
    OldHandle();
    virtual ~OldHandle();

    void initialize(GMainLoop* mainloop);

    // Signal
    void sendThresholdChangedSignal(string& prev, string& cur);

private:
    // APIs : Subscription only
    bool getCloseAppId(LSMessage &message);
    bool getCustomThreshold(LSMessage &message);

    // APIs : No subscription
    bool getCurrentMemState(LSMessage &message);
    bool setRequiredMemory(LSMessage &message);

    // APIs : Unsupported
    bool getGroupInfo(LSMessage &message);
    bool getPolicy(LSMessage &message);
    bool getUnitList(LSMessage &message);
    bool startMemNotifier(LSMessage &message);
    bool sendLowMemPopupTest(LSMessage &message);
    bool getEFS(LSMessage &message);
    bool setMriHelper(LSMessage &message);

    static const string NAME_SERVICE;
    static const string NAME_SIGNAL;
    static const LSSignal TABLE_SIGNAL[2];

};

#endif /* LUNA_SERVICE_OLDHANDLE_H_ */
