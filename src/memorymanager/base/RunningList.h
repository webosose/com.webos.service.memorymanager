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

#ifndef BASE_RUNNINGLIST_H_
#define BASE_RUNNINGLIST_H_

#include <iostream>
#include <vector>

#include "interface/IClassName.h"

#include "Application.h"

using namespace std;

class RunningList : public IClassName {
public:
    RunningList();
    virtual ~RunningList();

    Application& push();
    Application& back();
    Application& front();

    void sort();
    vector<Application>::iterator find(const string& instanceId, const string& appId);

    vector<Application>& getRunningList();
    int getCount();
    string getForegroundAppId();

    bool isExist(int pid);
    bool isExist(string appId);
    bool isEmpty();

    void print();

private:
    vector<Application>::iterator findByAppId(const string& appId);
    vector<Application>::iterator findByInstanceId(const string& instanceId);

    vector<Application> m_applications;
};

#endif /* BASE_RUNNINGLIST_H_ */
