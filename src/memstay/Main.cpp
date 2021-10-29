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
#include <glib.h>
#include <getopt.h>
#include <iostream>

#include "MemStay.h"
#include "util/Proc.h"

using namespace std;

void help()
{
    cout << endl ;
    cout << "memstay [OPTIONS...]" << endl ;
    cout << "    -h|--help  : Show this help" << endl;
    cout << "    -s|--swap-usage <n> : Set swap usage percentage(%)" << endl;
    cout << "    -m|--memory-usage <n> : Set memory Usage Percentage(%)" << endl << endl;
}

int main(int argc, char** argv)
{
    int interval = 10;
    int unit = 1;
    int memUsageRate = -1; //locked mem usage raet
    int swapUsageRate = -1; // swap-outed mem usage rate
    int opt;

    const char *const short_options = "hs:m:";
    const struct option long_options[] = {
        { "help", no_argument, nullptr, 'h' },
        { "swap-usage", required_argument, nullptr, 's' },
        { "memory-usage", required_argument, nullptr, 'm' },
        { nullptr, no_argument, nullptr, 0 }
    };

    while (true) {
        const auto opt = getopt_long(argc, argv, short_options, long_options, nullptr);

        if (opt == -1)
            break;

        switch (opt) {
            case 's':
                swapUsageRate = stoi(optarg);
                if ( swapUsageRate < 0 || swapUsageRate > 100) {
                    cout << "[memstay] swapUsage should be percentage value(0-100)" << endl;
                }
                break;
            case 'm':
                memUsageRate = stoi(optarg);
                if ( memUsageRate < 0 || memUsageRate > 100) {
                    cout << "[memstay] memUsage should be percentage value(0-100)" << endl;
                }
                break;
            case 'h':
                help();
                return 0;
            default:
                break;
        }
    }

    if ( memUsageRate == -1 && swapUsageRate == -1) {
        help();
        return 0;
    }

    MemStay::getInstance().setInterval(interval);
    MemStay::getInstance().setUnit(unit);
    MemStay::getInstance().setMemUsageRate(memUsageRate / 100.0);
    MemStay::getInstance().setSwapUsageRate(swapUsageRate / 100.0);

    MemStay::getInstance().configure();

    GMainLoop* mainLoop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(mainLoop);
    return 0;
}
