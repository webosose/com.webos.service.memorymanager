// Copyright (c) 2019-2020 LG Electronics, Inc.
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

#include "util/JValueUtil.h"

bool JValueUtil::getValue(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey, JValue& value)
{
    if (!json)
        return false;
    if (!json.hasKey(firstKey))
        return false;
    if (!json[firstKey].isObject())
        return false;
    return getValue(json[firstKey], secondKey, thirdKey, value);
}

bool JValueUtil::getValue(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey, string& value)
{
    if (!json)
        return false;
    if (!json.hasKey(firstKey))
        return false;
    if (!json[firstKey].isObject())
        return false;
    return getValue(json[firstKey], secondKey, thirdKey, value);
}

bool JValueUtil::getValue(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey, int& value)
{
    if (!json)
        return false;
    if (!json.hasKey(firstKey))
        return false;
    if (!json[firstKey].isObject())
        return false;
    return getValue(json[firstKey], secondKey, thirdKey, value);
}

bool JValueUtil::getValue(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey, bool& value)
{
    if (!json)
        return false;
    if (!json.hasKey(firstKey))
        return false;
    if (!json[firstKey].isObject())
        return false;
    return getValue(json[firstKey], secondKey, thirdKey, value);
}

bool JValueUtil::getValue(const JValue& json, const string& mainKey, const string& subKey, JValue& value)
{
    if (!json)
        return false;
    if (!json.hasKey(mainKey))
        return false;
    if (!json[mainKey].isObject())
        return false;
    return getValue(json[mainKey], subKey, value);
}

bool JValueUtil::getValue(const JValue& json, const string& mainKey, const string& subKey, string& value)
{
    if (!json)
        return false;
    if (!json.hasKey(mainKey))
        return false;
    if (!json[mainKey].isObject())
        return false;
    return getValue(json[mainKey], subKey, value);
}

bool JValueUtil::getValue(const JValue& json, const string& mainKey, const string& subKey, int& value)
{
    if (!json)
        return false;
    if (!json.hasKey(mainKey))
        return false;
    if (!json[mainKey].isObject())
        return false;
    return getValue(json[mainKey], subKey, value);
}

bool JValueUtil::getValue(const JValue& json, const string& mainKey, const string& subKey, bool& value)
{
    if (!json)
        return false;
    if (!json.hasKey(mainKey))
        return false;
    if (!json[mainKey].isObject())
        return false;
    return getValue(json[mainKey], subKey, value);
}

bool JValueUtil::getValue(const JValue& json, const string& key, JValue& value)
{
    if (!json)
        return false;
    if (!json.hasKey(key))
        return false;
    value = json[key];
    return true;
}

bool JValueUtil::getValue(const JValue& json, const string& key, string& value)
{
    if (!json)
        return false;
    if (!json.hasKey(key))
        return false;
    if (!json[key].isString())
        return false;
    if (json[key].asString(value) != CONV_OK) {
        value = "";
        return false;
    }
    return true;
}

bool JValueUtil::getValue(const JValue& json, const string& key, int& value)
{
    if (!json)
        return false;
    if (!json.hasKey(key))
        return false;
    if (!json[key].isNumber())
        return false;
    if (json[key].asNumber<int>(value) != CONV_OK) {
        value = 0;
        return false;
    }
    return true;
}

bool JValueUtil::getValue(const JValue& json, const string& key, bool& value)
{
    if (!json)
        return false;
    if (!json.hasKey(key))
        return false;
    if (!json[key].isBoolean())
        return false;
    if (json[key].asBool(value) != CONV_OK) {
        value = false;
        return false;
    }
    return true;
}

bool JValueUtil::hasKey(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey)
{
    if (!json.isObject())
        return false;
    if (!json.hasKey(firstKey))
        return false;
    if (!secondKey.empty() && (!json[firstKey].isObject() || !json[firstKey].hasKey(secondKey)))
        return false;
    if (!thirdKey.empty() && (!json[firstKey][secondKey].isObject() || !json[firstKey][secondKey].hasKey(thirdKey)))
        return false;
    return true;
}
