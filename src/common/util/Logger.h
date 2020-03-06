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

#ifndef UTIL_LOGGER_H_
#define UTIL_LOGGER_H_

#include <iostream>

#include "Fifo.h"

using namespace std;

enum LogLevel {
    LogLevel_VERBOSE,
    LogLevel_DEBUG,
    LogLevel_NORMAL,
    LogLevel_WARNING,
    LogLevel_ERROR,
};

enum LogType {
    LogType_CONSOLE,
    LogType_PMLOG
};

class Logger {
public:
    static void verbose(const string& msg, const string& name = "");
    static void debug(const string& msg, const string& name = "");
    static void normal(const string& msg, const string& name = "");
    static void warning(const string& msg, const string& name = "");
    static void error(const string& msg, const string& name = "");

    static Logger& getInstance()
    {
        static Logger _instance;
        return _instance;
    }

    virtual ~Logger();

    void setLevel(enum LogLevel level);
    void setType(enum LogType type);

private:
    static string& convertLevel(enum LogLevel& level);

    Logger();

    void write(const string& msg, const string& name, enum LogLevel level = LogLevel_DEBUG);
    void writeConsole(string& log, enum LogLevel& level);

    enum LogLevel m_level;
    enum LogType m_type;
};

#endif /* UTIL_LOGGER_H_ */
