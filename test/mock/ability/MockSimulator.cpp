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

#include "simulator.h"
#include "MockGlobalResult.h"

namespace OHOS {
namespace AbilityRuntime {

class MockSimulator : public Simulator {
public:
    MockSimulator() = default;
    ~MockSimulator() = default;
    int64_t StartAbility(const std::string &abilitySrcPath, TerminateCallback callback,
        const std::string &abilityName) override
    {
        int ret = 1;
        return ret;
    }

    void TerminateAbility(int64_t abilityId) override
    {
        g_terminateAbility = true;
    }

    void UpdateConfiguration(const AppExecFwk::Configuration &config) override
    {
        g_updateConfiguration = true;
    }

    void SetMockList(const std::map<std::string, std::string> &mockList) override
    {
        g_setAbilityMockList = true;
    }

    void SetHostResolveBufferTracker(ResolveBufferTrackerCallback cb) override
    {
        g_setHostResolveBufferTracker = true;
    }

    Options option_;
};

std::shared_ptr<Simulator> Simulator::Create(const Options &options)
{
    auto simulator = std::make_unique<MockSimulator>();
    if (simulator) {
        simulator->option_ = options;
        return simulator;
    }
    return nullptr;
}
}
}