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

#include <iostream>
#include <glib.h>

#include "MemoryManager.h"
#include "util/Logger.h"

#define LOG_NAME "MAIN"

using namespace std;

int main(int argc, char** argv)
{
    Logger::verbose("start main program", LOG_NAME);
    MemoryManager::getInstance().initialize();
    MemoryManager::getInstance().run();
    Logger::verbose("end main program", LOG_NAME);
    return 1;
}

