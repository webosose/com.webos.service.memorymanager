/* @@@LICENSE
 *
 * Copyright (c) 2018 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */

#include <iostream>
#include <glib.h>

#include "MemoryManager.h"
#include "util/Logger.h"

#define LOG_NAME "MAIN"

using namespace std;

int main(int argc, char** argv)
{
    Logger::verbose("start main program", LOG_NAME);
    MemoryManager::getInstance().initialize();
    MemoryManager::getInstance().run();
    Logger::verbose("end main program", LOG_NAME);
    return 1;
}

