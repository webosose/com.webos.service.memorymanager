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

#include <glib.h>
#include <proc/readproc.h>
#include <stdlib.h>
#include <array>
#include <memory>
#include <regex>
#include <boost/algorithm/string.hpp>

#include "LinuxProcess.h"

#define BUF_SIZE 1024

const string LinuxProcess::CLASS_NAME = "LinuxProcess";

bool LinuxProcess::forkSyncProcess(const char **argv, const char **envp)
{
    GError* gerr = NULL;
    GSpawnFlags flags = (GSpawnFlags) (G_SPAWN_STDOUT_TO_DEV_NULL
                                       | G_SPAWN_STDERR_TO_DEV_NULL
                                       | G_SPAWN_CHILD_INHERITS_STDIN);

    if (argv) {
        string cmd = "";
        for (char const **p = argv; *p; p++) {
            cmd += string(*p) + " ";
        }
        Logger::verbose("Try to run cmd (" + cmd + ")", CLASS_NAME);
    }

    gboolean result = g_spawn_sync(
        NULL,
        const_cast<char**>(argv),  // cmd arguments
        const_cast<char**>(envp),  // environment variables
        flags,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        &gerr
    );
    if (gerr) {
        Logger::error("Failed to folk: errorText: " + string(gerr->message), CLASS_NAME);
        g_error_free(gerr);
        gerr = NULL;
        return false;
    }
    if (!result) {
        Logger::error("Failed to folk", CLASS_NAME);
        return false;
    }

    return true;
}

pid_t LinuxProcess::forkAsyncProcess(const char **argv, const char **envp)
{
    GPid pid = -1;
    GError* gerr = NULL;
    GSpawnFlags flags = (GSpawnFlags) (G_SPAWN_STDOUT_TO_DEV_NULL
                                       | G_SPAWN_STDERR_TO_DEV_NULL
                                       | G_SPAWN_DO_NOT_REAP_CHILD
                                       | G_SPAWN_CHILD_INHERITS_STDIN);

    if (argv) {
        string cmd = "";
        for (char const **p = argv; *p; p++) {
            cmd += string(*p) + " ";
        }
        Logger::verbose("Try to run cmd (" + cmd + ")", CLASS_NAME);
    }

    gboolean result = g_spawn_async_with_pipes(
        NULL,
        const_cast<char**>(argv),  // cmd arguments
        const_cast<char**>(envp),  // environment variables
        flags,
        NULL,
        NULL,
        &pid,
        NULL,
        NULL,
        NULL,
        &gerr
    );
    if (gerr) {
        Logger::error("Failed to folk: pid: " + to_string(pid)
                      + " errorText: " + string(gerr->message), CLASS_NAME);
        g_error_free(gerr);
        gerr = NULL;
        return -1;
    }
    if (!result) {
        Logger::error("Failed to folk: pid: " + to_string(pid), CLASS_NAME);
        return -1;
    }

    return pid;
}

string LinuxProcess::getStdoutFromCmd(const string& cmd)
{
    Logger::verbose("RunCmd (" + cmd + ")", CLASS_NAME);

    array<char, BUF_SIZE> buf;
    string result = "";
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        return "";
    }
    while (fgets(buf.data(), buf.size(), pipe.get()) != nullptr) {
        result += buf.data();
    }

    boost::trim(result);
    Logger::verbose("Result (" + result + ")", CLASS_NAME);
    return result;
}
