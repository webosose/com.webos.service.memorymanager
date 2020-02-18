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

#ifndef SWAP_SWAPMANAGER_H_
#define SWAP_SWAPMANAGER_H_

#include <iostream>

#include "base/IManager.h"

using namespace std;

enum SwapMode {
    SwapMode_NO = 0,
    SwapMode_MEMORY,
    SwapMode_FULL
};

class SwapManagerListener {
public:
    SwapManagerListener() {};
    virtual ~SwapManagerListener() {};

};

class SwapManager : public IManager<SwapManagerListener> {
public:
    static SwapManager& getInstance()
    {
        static SwapManager s_instance;
        return s_instance;

    }

    virtual ~SwapManager();

    // IManager
    void initialize(GMainLoop* mainloop);

private:
    SwapManager();

    enum SwapMode m_mode;
    int m_size;
    string m_partition;

    // Enhanced Flash Swap
    static const char* EFS_CTL_BIN;
};

#endif /* SWAP_SWAPMANAGER_H_ */
