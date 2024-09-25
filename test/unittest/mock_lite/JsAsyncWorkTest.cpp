/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "js_async_work.h"
#define private public
#include "AsyncWorkManager.h"

namespace {
    void AddHandler([[maybe_unused]] void *data)
    {
        // for test add task, do nothing
    }

    TEST(JsAsyncWorkTest, DispatchAsyncWorkTest)
    {
        OHOS::ACELite::JsAsyncWork::DispatchAsyncWork(AddHandler, nullptr);
        EXPECT_EQ(AsyncWorkManager::GetInstance().workList.size(), 1); // 1 element in list
        AsyncWorkManager::GetInstance().ClearAllAsyncWork();
        EXPECT_EQ(AsyncWorkManager::GetInstance().workList.size(), 0); // 0 element in list
    }
}