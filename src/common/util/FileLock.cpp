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

#include "FileLock.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "Logger.h"

#define LOG_NAME "FileLock"

FileLock::FileLock(string path)
{
    m_fd = open(path.c_str(), O_RDWR | O_CREAT);
    if (m_fd < 0) {
        Logger::error("File open error - " + path, LOG_NAME);
        Logger::error(strerror(errno), LOG_NAME);
    }
}

FileLock::~FileLock()
{
    if (m_fd < 0)
        close(m_fd);
}

bool FileLock::trylock()
{
    if (m_fd < 0)
        return false;
    if (lockf(m_fd, F_TLOCK, 0) == -1)
        return false;
    return true;
}

bool FileLock::isLocked()
{
    if (m_fd < 0)
        return true;
    return (lockf(m_fd, F_TEST, 0) != 0);
}

void FileLock::lock()
{
    if (m_fd < 0)
        return;
    lockf(m_fd, F_LOCK, 0);
}

void FileLock::unlock()
{
    if (m_fd < 0)
        return;
    lockf(m_fd, F_ULOCK, 0);
}

