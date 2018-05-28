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

#include "MemStay.h"
#include "util/Proc.h"

using namespace std;

int main(int argc, char** argv)
{
    int target = 200;
    int interval = 1;
    int unit = 1;
    bool free = false;

    if (argc == 1) {
        long total, available;
        Proc::getMemoryInfo(total, available);
        cerr << "[memstay] Three parameters are needed (optional)" << endl;
        cerr << "[memstay] #1 : Allocation Target (mb) - Default 200mb"  << endl;
        cerr << "[memstay] #2 : Allocation Interval (ms) - Default 1ms"  << endl;
        cerr << "[memstay] #3 : Allocation Unit (mb) - Default 1mb" << endl;
        cerr << "[memstay] #4 : Enable free operation - Default 'false'" << endl;
        cerr << "[memstay] #5 : Total(" << total << ") Free(" << available << ")" << endl;
        return 0;
    }
    if (argc >= 2) {
        target = atoi(argv[1]);
    }
    if (argc >= 3) {
        interval = atoi(argv[2]);
    }
    if (argc >= 4) {
        unit = atoi(argv[3]);
    }
    if (argc >= 5 && strcmp(argv[4], "free") == 0) {
        free = true;
    }

    cout << "[memstay] Allocation : target(" << argv[1] << "MB) / "
         << "interval(" << argv[2] << "ms) / "
         << "unit(" << argv[3] << "MB) / "
         << (free ? "free(enabled)" : "free(disabled)") << endl;

    MemStay::getInstance().setTarget(target);
    MemStay::getInstance().setInterval(interval);
    MemStay::getInstance().setUnit(unit);
    MemStay::getInstance().setFree(free);
    MemStay::getInstance().configure();

    GMainLoop* mainLoop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainLoop);
}

