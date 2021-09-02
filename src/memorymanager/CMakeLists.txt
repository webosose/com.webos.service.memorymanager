# Copyright (c) 2018-2021 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

include(FindPkgConfig)

pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GLIB2_CFLAGS_OTHER})

pkg_check_modules(LUNASERVICE2 REQUIRED luna-service2)
include_directories(${LUNASERVICE2_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE2_CFLAGS_OTHER})

pkg_check_modules(LUNASERVICE2CPP REQUIRED luna-service2++)
include_directories(${LUNASERVICE2CPP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${LUNASERVICE2CPP_CFLAGS_OTHER})

pkg_check_modules(PBNJSON_C REQUIRED pbnjson_c)
include_directories(${PBNJSON_C_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PBNJSON_C_CFLAGS_OTHER})

pkg_check_modules(PBNJSON_CPP REQUIRED pbnjson_cpp)
include_directories(${PBNJSON_CPP_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PBNJSON_CPP_CFLAGS_OTHER})

pkg_check_modules(PMLOG REQUIRED PmLogLib)
include_directories(${PMLOG_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PMLOG_CFLAGS_OTHER})

find_package(Boost REQUIRED COMPONENTS regex system filesystem)
include_directories(${Boost_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${Boost_CFLAGS_OTHER})

pkg_check_modules(PROCPS REQUIRED libprocps)
include_directories(${PROCPS_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${PROCPS_CFLAGS})

pkg_check_modules(GIO_UNIX REQUIRED gio-unix-2.0)
include_directories(${GIO_UNIX_INCLUDE_DIRS})
webos_add_compiler_flags(ALL ${GIO_UNIX_CFLAGS})

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MMBus.c
    COMMAND gdbus-codegen --pragma-once
                          --generate-c-code=${CMAKE_CURRENT_BINARY_DIR}/MMBus
                          --c-namespace MMBus
                          --interface-prefix com.webos.MemoryManager1.
                          ${PROJECT_SOURCE_DIR}/files/dbus/MemoryManager1.xml

    DEPENDS ${PROJECT_SOURCE_DIR}/files/dbus/MemoryManager1.xml
    COMMENT "Generate D-Bus 'MemroyManager1.xml"
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Environment
set(BIN_NAME memorymanager)
file(GLOB_RECURSE SRC_COMMON ${PROJECT_SOURCE_DIR}/src/common/*.cpp)
file(GLOB_RECURSE SRC_MEMORYMANAGER ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
set(SRC_DBUS ${CMAKE_CURRENT_BINARY_DIR}/MMBus.c)

# Compile
webos_add_compiler_flags(ALL CXX -std=c++0x)
include_directories(${PROJECT_BINARY_DIR}/Configured/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src/common)
add_executable(${BIN_NAME} ${SRC_COMMON} ${SRC_MEMORYMANAGER} ${SRC_DBUS})

# Link
set(LIBS
    ${GLIB2_LDFLAGS}
    ${LUNASERVICE2_LDFLAGS}
    ${LUNASERVICE2CPP_LDFLAGS}
    ${PMLOG_LDFLAGS}
    ${Boost_LIBRARIES}
    ${PBNJSON_C_LDFLAGS}
    ${PBNJSON_CPP_LDFLAGS}
    ${PROCPS_LDFLAGS}
    ${GIO_UNIX_LDFLAGS}
)
target_link_libraries(${BIN_NAME} ${LIBS})

# Install
install(TARGETS ${BIN_NAME} DESTINATION ${WEBOS_INSTALL_SBINDIR})
webos_build_configured_file(files/launch/memorymanager.conf SYSCONFDIR init)
webos_build_configured_file(files/launch/memorymanager.service SYSCONFDIR systemd/system)
