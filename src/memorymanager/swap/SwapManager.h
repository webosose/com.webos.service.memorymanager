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

#include "interface/IManager.h"
#include "interface/IClassName.h"
#include "interface/ISingleton.h"

using namespace std;

enum SwapMode {
    SwapMode_NO = 0,
    SwapMode_MEMORY,
    SwapMode_FULL
};

class SwapManager : public ISingleton<SwapManager>,
                    public IClassName {
friend class ISingleton<SwapManager>;
public:
    virtual ~SwapManager();

    // IManager
    void initialize(GMainLoop* mainloop);

private:
    static const char* PATH_EFS_CORE_MODULE;
    static const char* PATH_EFS_ADAPTOR_MODULE;
    static const char* NAME_EFS_CTL;

    SwapManager();

    enum SwapMode m_mode;
    int m_size;
    string m_partition;
};

#endif /* SWAP_SWAPMANAGER_H_ */
