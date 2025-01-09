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
#include "secodeFuzz.h"
#include "common.h"
#include "CommandParse.h"
#include "CommandLineInterface.h"
#define private public
#include "CommandParser.h"
using namespace fuzztest;

namespace {
std::map<std::string, std::string> richDataMap = {
    {"BackClicked", ""},
    {"inspector", ""},
    {"inspectorDefault", ""},
    {"ColorMode", R"({"ColorMode":"dark"})"},
    {"Orientation", R"({"Orientation":"landscape","screenDensity":480,"width":2340,
        "height":1080,"originWidth":2340,"originHeight":1080})"},
    {"ResolutionSwitch", R"({"originWidth":1080,"originHeight":2340,"width":1080,
        "height":2340,"screenDensity":480})"},
    {"CurrentRouter", ""},
    {"ReloadRuntimePage", R"({"ReloadRuntimePage":"aaa"})"},
    {"FontSelect", R"({"FontSelect":true})"},
    {"MemoryRefresh", R"({"jsCode":"UEFOREEAAAAAAAAAAAAAA+wDAADEAAAAFQAAAAMoDAAA=\r\n",
        "propertyVariable":[],"viewID":"1","offset":{"line":11,"column":9},
        "globalVariable":[],"slot":0,"type":"UpdateComponent","parentID":2})"},
    {"LoadDocument", R"({"url":"pages/Index","className":"Index","previewParam":{"width":1080,
        "height":2340,"locale":"zh_CN","colorMode":"light","orientation":"portrait",
        "deviceType":"phone","dpi":480}})"},
    {"FastPreviewMsg", ""},
    {"DropFrame", R"({"frequency":1000})"},
    {"KeyPress", R"({"isInputMethod":false,"keyCode":2033,"keyAction":0,"pressedCodes":[2033]})"},
    {"PointEvent", R"({"x":365,"y":1071,"duration":"","button":-1,"action": 3,"axisValues":[0,0,0,0],
        "sourceType":1,"sourceTool": 7,"pressedButtons":[0,1]})"},
    {"FoldStatus", R"({"FoldStatus":"fold","width":1080,"height":2504})"},
    {"AvoidArea", R"({"topRect":{"posX":0,"posY":0,"width":2340,"height":117},"bottomRect":{"posX":
        0,"posY":0,"width":0,"height":0},"leftRect":{"posX":0,"posY":0,"width":0,"height":0},
        "rightRect":{"posX":0,"posY":0,"width":2340,"height":84}})"},
    {"AvoidAreaChanged", ""},
    {"AvoidAreaChanged", ""},
    {"ScreenShot", ""},
    {"StartVideoRecord", ""},
    {"StopVideoRecord", ""}
};

TEST(RichCommandParseFuzzTest, test_command)
{
    std::cout << "--> RichCommandParseFuzzTest for rich start <--" << std::endl;
    DT_FUZZ_START(0, TEST_TIMES, (char*)"RichCommandParseFuzzTest", 0)
    {
        CommandParse parse;
        CommandParser::GetInstance().deviceType = "phone";
        parse.CreateAndExecuteCommand(richDataMap);
    }
    DT_FUZZ_END()
    printf("end ---- RichCommandParseFuzzTest\r\n");
    if (DT_GetIsPass() == 0) {
        printf("test RichCommandParseFuzzTest is not ok\r\n");
    } else {
        printf("test RichCommandParseFuzzTest is ok\r\n");
    }
    std::cout << "--> RichCommandParseFuzzTest for rich end <--" << std::endl;
}
}
