// Copyright (c) 2020 LG Electronics, Inc.
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
//
#ifndef INTERFACE_ISINGLETON_H_
#define INTERFACE_ISINGLETON_H_

template <class T>
class ISingleton {
protected:
    ISingleton()
    {

    }

    virtual ~ISingleton()
    {

    };

public:
    static T* getInstance()
    {
        if (m_pInstance == NULL)
            m_pInstance = new T;

        return m_pInstance;
    }

    static void destroyInstance()
    {
        if (m_pInstance) {
            delete m_pInstance;
            m_pInstance = NULL;
        }
    }

private:
    static T* m_pInstance;
};

template <class T> T* ISingleton<T>::m_pInstance = NULL;

#endif /* INTERFACE_ISINGLETON_H_ */
