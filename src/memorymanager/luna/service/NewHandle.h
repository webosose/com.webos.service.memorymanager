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

#ifndef LUNA_SERVICE_NEWHANDLE_H_
#define LUNA_SERVICE_NEWHANDLE_H_

#include <iostream>
#include <luna-service2/lunaservice.hpp>
#include <pbnjson.hpp>
#include <glib.h>

using namespace std;
using namespace LS;
using namespace pbnjson;

class NewHandle : public Handle {
public:
    NewHandle();
    virtual ~NewHandle();

    void initialize(GMainLoop* mainloop);

    // signal
    void sendLevelChangedSignal(string& prev, string& cur);

private:
    bool getManagerEvent(LSMessage& message);
    bool getMemoryStatus(LSMessage& message);
    bool requireMemory(LSMessage& message);

    static const string NAME_SERVICE;
    static const string NAME_SIGNAL;
    static const LSSignal TABLE_SIGNAL[2];

};

#endif /* LUNA_SERVICE_NEWHANDLE_H_ */
