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

#include "PublicMethods.h"

uint32_t PublicMethods::Ulltoa(uintptr_t value, int8_t (&rstStr)[MAX_ITOA_BIT])
{
    const int32_t RADIX_HEXADECIMAL = 16;
    const int32_t dividendLength = 10;

    auto remainder = value;
    int8_t strSpace[MAX_ITOA_BIT] = {0};
    int8_t* curPoint = strSpace;
    int64_t dividend = 0;
    uint32_t rstLength = 0;
    while (remainder || curPoint == strSpace) {
        rstLength++;
        dividend = remainder % RADIX_HEXADECIMAL;
        remainder = remainder / RADIX_HEXADECIMAL;
        if (remainder == 0) {
            break;
        }
        if (dividend < dividendLength) { // Converted to hexadecimal
            *curPoint++ = dividend + '0';
        } else {
            *curPoint++ = dividend + 'a' - dividendLength;
        }
    }
    int8_t* tmpRstStr = rstStr;
    while (curPoint > strSpace) {
        *tmpRstStr++ = *--curPoint;
    }
    *tmpRstStr = 0;
    return rstLength;
}
