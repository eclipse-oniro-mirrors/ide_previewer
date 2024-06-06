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

#include <string>
#include <map>
#include <gtest/gtest.h>
#include "CommandParse.h"
#include "secodeFuzz.h"
#include "CommandParse.h"
#include "common.h"
#include "CommandLineInterface.h"
#define private public
#include "CommandParser.h"
using namespace std;
using namespace fuzztest;

std::map<std::string, std::string> liteDataMap = {
    {"Power", R"({"Power":1.0})"},
    {"Volume", R"({"Volume":90})"},
    {"Barometer", R"({"Barometer":1.0})"},
    {"Location", R"({"latitude":"10.9023142","longitude":"56.3043242"})"},
    {"KeepScreenOnState", R"({"KeepScreenOnState":true})"},
    {"WearingState", R"({"WearingState":true})"},
    {"BrightnessMode", R"({"BrightnessMode":1})"},
    {"ChargeMode", R"({"ChargeMode":1})"},
    {"Brightness", R"({"Brightness":100})"},
    {"HeartRate", R"({"HeartRate":100})"},
    {"StepCount", R"({"StepCount":10025})"},
    {"DistributedCommunications", R"({"DeviceId":"68-05-CA-90-9A-66","bundleName":"com.test.demo",
        "abilityName":"hello","message":"{ action:'GET_WEATHER',city:'HangZhou' }"})"},
    {"CrownRotate", R"({"rotate":150})"}
};

TEST(LiteCommandParseFuzzTest, test_command)
{
    std::cout << "--> LiteCommandParseFuzzTest for rich start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"LiteCommandParseFuzzTest", 0)
    {
        CommandParse parse;
        CommandParser::GetInstance().deviceType = "liteWearable";
        parse.CreateAndExecuteCommand(liteDataMap);
    }
    DT_FUZZ_END()
    printf("end ---- LiteCommandParseFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test LiteCommandParseFuzzTest is not ok\r\n");
    } else {
        printf("test LiteCommandParseFuzzTest is ok\r\n");
    }
    std::cout << "--> LiteCommandParseFuzzTest for rich end <--" << std::endl;
}