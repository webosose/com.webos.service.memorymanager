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

    bool setMode(const string mode);
    void setPartition(const string partition);
    bool setSize(const int size);

private:
    static const string EFS_CTL_BIN;
    static const string EFS_MAPPER_PATH;
    static const string EFS_PARTLABEL;

    string findPartitionByPartLabel(const string partLabel);
    bool createEFS(const enum SwapMode mode, const string partition, const int size);
    bool createSwap(const string device);
    SwapManager();

    enum SwapMode m_mode;
    string m_partition;
    int m_size;
};

#endif /* SWAP_SWAPMANAGER_H_ */
