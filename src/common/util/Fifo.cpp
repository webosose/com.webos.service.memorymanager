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

#include "Fifo.h"

#include <error.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Logger.h"

#define LOG_NAME "FIFO"

Fifo::Fifo()
    : m_fd(-1)
    , m_readonly(true)
{

}

Fifo::~Fifo()
{
    close();
}

bool Fifo::open(string path, bool readonly)
{
    m_readonly = readonly;
    mkfifo(path.c_str(), 0666);
    m_fd = ::open(path.c_str(), O_RDWR);
    if (m_fd < 0) {
        Logger::error(strerror(errno), LOG_NAME);
        return false;
    }
    return true;
}

bool Fifo::isClose()
{
    return (m_fd <= 0);
}

void Fifo::close()
{
    if (m_fd > 0)
        ::close(m_fd);
    m_fd = -1;
}

int Fifo::send(const void* buffer, int size)
{
    if (m_readonly || m_fd < 0) {
        Logger::error("Invalid write operation", LOG_NAME);
        return -1;
    }
    return ::write(m_fd, buffer, size);
}

int Fifo::receive(void *buffer, int size)
{
    if (!m_readonly || m_fd < 0) {
        Logger::error("Invalid read operation", LOG_NAME);
        return -1;
    }
    return ::read(m_fd, buffer, size);
}
