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

#ifndef UTIL_LINUXPROCESS_H_
#define UTIL_LINUXPROCESS_H_

#include <iostream>
#include <fcntl.h>

#include "util/Logger.h"

using namespace std;

class LinuxProcess {
public:
    static pid_t forkAsyncProcess(const char **argv, const char **envp);
    static bool forkSyncProcess(const char **argv, const char **envp);
    static string getStdoutFromCmd(const string& cmd);

private:
    static const string CLASS_NAME;

    LinuxProcess() {}
    virtual ~LinuxProcess() {}
};

#endif /* UTIL_LINUXPROCESS_H_ */
