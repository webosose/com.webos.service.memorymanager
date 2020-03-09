/* @@@LICENSE
 *
 * Copyright (c) 2020 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */

#ifndef UTIL_LSMESSAGEUTIL_H_
#define UTIL_LSMESSAGEUTIL_H_

#include <iostream>
#include <luna-service2/lunaservice.hpp>

using namespace std;

class LSMessageUtil {
public:
    LSMessageUtil() {};
    virtual ~LSMessageUtil() {};

    static string getSessionId(LSMessage *reply);
};

#endif /* UTIL_LSMESSAGEUTIL_H_ */
