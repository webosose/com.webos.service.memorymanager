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

#include "Docker.h"
#include "LinuxProcess.h"

const string Docker::PATH_DOCKER_CMD = "/usr/bin/docker";
const string Docker::INSPECT_PID = "{{.State.Pid}}";
const string Docker::INSPECT_CONTAINER_ID = "{{.Id}}";

string Docker::inspect(const string& sessionId, const string& item)
{
    /*
     * cmd             : /usr/bin/docker inspect -f [item] [sesssionId]
     * expected result : value in [item] field
     */
    string cmd = Docker::PATH_DOCKER_CMD + " inspect -f '" + item + "' " + sessionId;
    string result = LinuxProcess::getStdoutFromCmd(cmd);
    return result;
}
