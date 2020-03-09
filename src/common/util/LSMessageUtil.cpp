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

#include "LSMessageUtil.h"

string LSMessageUtil::getSessionId(LSMessage *reply)
{
#if defined(WEBOS_TARGET_DISTRO_WEBOS_AUTO)
    return LSMessageGetSessionId(reply);
#else
    return "";
#endif
}
