/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ENDIANUTIL_H
#define ENDIANUTIL_H

#include <cstdint>
#include <type_traits>

namespace EndianUtil {
    bool IsBigEndian();

    template <class T, class = typename std::enable_if<std::is_integral<T>::value>::type>
    T ToNetworkEndian(T data)
    {
        if (IsBigEndian()) {
            return data;
        }

        union {
            T data;
            unsigned char buffer[sizeof(T)];
        } source;

        union {
            T data;
            unsigned char buffer[sizeof(T)];
        } dest;

        source.data = data;
        for (std::size_t i = 0; i < sizeof(T); i++) {
            dest.buffer[i] = source.buffer[sizeof(T) - i - 1];
        }
        return dest.data;
    }
}; // namespace EndianUtil

#endif // ENDIANUTIL_H
