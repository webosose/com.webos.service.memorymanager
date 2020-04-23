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

#include "Logger.h"

#include <string.h>

void Logger::verbose(const string& msg, const string& name)
{
    getInstance().write(msg, name, LogLevel_VERBOSE);
}

void Logger::debug(const string& msg, const string& name)
{
    getInstance().write(msg, name, LogLevel_DEBUG);
}

void Logger::normal(const string& msg, const string& name)
{
    getInstance().write(msg, name, LogLevel_NORMAL);
}

void Logger::warning(const string& msg, const string& name)
{
    getInstance().write(msg, name, LogLevel_WARNING);
}

void Logger::error(const string& msg, const string& name)
{
    getInstance().write(msg, name, LogLevel_ERROR);
}

string& Logger::convertLevel(enum LogLevel& level)
{
    static string verbose = "VERBOSE";
    static string debug = "DEBUG";
    static string normal = "NORMAL";
    static string warning = "WARNING";
    static string error = "ERROR";

    switch(level) {
    case LogLevel_VERBOSE:
        return verbose;

    case LogLevel_DEBUG:
        return debug;

    case LogLevel_NORMAL:
        return normal;

    case LogLevel_WARNING:
        return warning;

    case LogLevel_ERROR:
       return error;
    }
    return debug;
}

Logger::Logger()
    : m_level(LogLevel_VERBOSE)
    , m_type(LogType_CONSOLE)
{
}

Logger::~Logger()
{
}

void Logger::setLevel(enum LogLevel level)
{
    m_level = level;
}

void Logger::setType(enum LogType type)
{
    m_type = type;
}

void Logger::write(const string& msg, const string& name, enum LogLevel level)
{
    if (level < m_level)
        return;

    string txt = "[" + convertLevel(level) + "][" + (name.empty() ? "UNKNOWN" : name)  + "] " + msg;

    switch (m_type) {
    case LogType_CONSOLE:
        writeConsole(txt, level);
        break;

    default:
        cerr << "Unsupported Log Type" << endl;
        break;
    }
}

void Logger::writeConsole(string& log, enum LogLevel& level)
{
    switch(level) {
    case LogLevel_VERBOSE:
    case LogLevel_DEBUG:
    case LogLevel_NORMAL:
        cout << log << endl;
        break;

    case LogLevel_WARNING:
    case LogLevel_ERROR:
        cerr << log << endl;
        break;
    }
}
