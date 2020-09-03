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

#ifndef LUNA_CLIENT_NOTIFICATIONMANAGER_H_
#define LUNA_CLIENT_NOTIFICATIONMANAGER_H_

#include "AbsClient.hpp"

#include <iostream>
#include <pbnjson.hpp>

using namespace std;
using namespace pbnjson;

class NotificationManager {
public:
    NotificationManager() {};
    virtual ~NotificationManager() {};

    static void createToast(string message, const string& sessionId);

private:
    static bool onCreateToast(LSHandle *sh, LSMessage *reply, void *ctx);


};

#endif /* LUNA_CLIENT_NOTIFICATIONMANAGER_H_ */
