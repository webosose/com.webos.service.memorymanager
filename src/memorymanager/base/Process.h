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

#ifndef BASE_PROCESS_H_
#define BASE_PROCESS_H_

#include <iostream>
#include <fstream>
#include <proc/readproc.h>
#include <stdlib.h>
#include <string.h>

#include "interface/IPrintable.h"
#include "interface/IClassName.h"

using namespace std;

class Process : public IPrintable,
                public IClassName {
public:
    static bool compareTid(const Process& a, const Process& b)
    {
        return a.getTid() > b.getTid();
    }

    static bool compareRss(const Process& a, const Process& b)
    {
        return a.getRss() > b.getRss();
    }

    static bool comparePss(const Process& a, const Process& b)
    {
        return a.getPss() > b.getPss();
    }

    Process();
    virtual ~Process();

    void update();

    void fromProc(proc_t& processInfo);

    void setPpid(int pid);
    int getPpid();

    void setTid(int tid);
    int getTid() const;

    void setCmd(string cmd);
    string& getCmd();

    void setSize(int size);
    int getSize();

    void setRss(int rss);
    int getRss() const;
    int getPss() const;

    void setShared(int shared);
    int getShared();

    void setText(int text);
    int getText();

    void setData(int data);
    int getData();

    virtual void print();
    virtual void print(JValue& json);

protected:
    int m_ppid;
    int m_tid;
    string m_cmd;
    int m_size;
    int m_rss;
    int m_shared;
    int m_text;
    int m_data;

};

#endif /* BASE_PROCESS_H_ */
