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

#include "KeyboardObjc.h"
#import <Carbon/Carbon.h>

short KeyboardObjc::GetKeyStateByKeyName(const std::string& keyName)
{
    NSString *dataStr = [NSString stringWithCString:keyName.c_str() encoding:NSUTF8StringEncoding];
    if ([dataStr isEqualToString:@"CapsLock"]) {
        short result = 1;
        @autoreleasepool {
            unsigned int modifiers = GetCurrentKeyModifiers();
            result = result && (modifiers & alphaLock);
        }
        return result;
    } else if ([dataStr isEqualToString:@"NumLock"]) {
        return 1;
    }
    return -1;
}