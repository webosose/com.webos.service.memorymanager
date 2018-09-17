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

#ifndef BASE_RUNNINGLIST_H_
#define BASE_RUNNINGLIST_H_

#include <iostream>
#include <vector>

#include "Application.h"

using namespace std;

class RunningList {
public:
    RunningList();
    virtual ~RunningList();

    bool push(Application& application);
    Application& back();
    Application& front();
    vector<Application>::iterator find(string appId);
    void sort();

    void resetPid();
    void removeZeroPid();

    vector<Application>& getRunningList();
    int getCount();
    string getForegroundAppId();

    bool isExist(string appId);
    bool isEmpty();

private:
    vector<Application> m_applications;

};

#endif /* BASE_RUNNINGLIST_H_ */