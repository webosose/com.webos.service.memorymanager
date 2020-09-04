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

#ifndef UTIL_JVALUEUTIL_H_
#define UTIL_JVALUEUTIL_H_

#include <iostream>
#include <map>
#include <pbnjson.hpp>

using namespace std;
using namespace pbnjson;

class JValueUtil {
public:
    JValueUtil() {}
    virtual ~JValueUtil() {}

    static bool getValue(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey, JValue& value);
    static bool getValue(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey, string& value);
    static bool getValue(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey, int& value);
    static bool getValue(const JValue& json, const string& firstKey, const string& secondKey, const string& thirdKey, bool& value);

    static bool getValue(const JValue& json, const string& mainKey, const string& subKey, JValue& value);
    static bool getValue(const JValue& json, const string& mainKey, const string& subKey, string& value);
    static bool getValue(const JValue& json, const string& mainKey, const string& subKey, int& value);
    static bool getValue(const JValue& json, const string& mainKey, const string& subKey, bool& value);

    static bool getValue(const JValue& json, const string& key, JValue& value);
    static bool getValue(const JValue& json, const string& key, string& value);
    static bool getValue(const JValue& json, const string& key, int& value);
    static bool getValue(const JValue& json, const string& key, bool& value);

    static bool hasKey(const JValue& json, const string& firstKey, const string& secondKey = "", const string& thirdKey = "");
};

#endif /* UTIL_JVALUEUTIL_H_ */
