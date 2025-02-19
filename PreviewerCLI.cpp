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

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "JsonReader.h"
#include "LocalSocket.h"

#include "utils/pandargs.h"

using namespace std;

static bool ToDouble(const char *str, double *result)
{
    char *end;
    *result = strtod(str, &end);
    return end != str && *end == '\0';
}

static Json2::Value parseOneArg(const char* arg)
{
    double value;
    if (ToDouble(arg, &value)) {
        return JsonReader::CreateNumber(value);
    }
    return JsonReader::CreateString(arg);
}

static Json2::Value parseArguments(const vector<string> &args)
{
    if (args.empty()) {
        return JsonReader::CreateNull();
    }
    const string &firstArg = args[0];
    if (firstArg[0] == '-') {
        size_t dashCount = firstArg.find_first_not_of('-');
        Json2::Value jsonObject = JsonReader::CreateObject();
        string key = args[0].substr(dashCount);
        vector<string> values;
        bool ret = false;
        for (size_t i = 1; i < args.size(); ++i) {
            size_t currentDashCount = args[i].find_first_not_of('-');
            if (currentDashCount == dashCount) {
                auto value = parseArguments(values);
                ret = value.IsValid() ? false : true;
                jsonObject.Add(key.c_str(), value);
                values.clear();
                key = args[i].substr(dashCount);
            } else if (currentDashCount < dashCount) {
                values.push_back(args[i]);
            } else {
                ret = true;
            }
            if (ret) {
                return Json2::Value();
            }
        }
        auto value = parseArguments(values);
        if (!value.IsValid()) {
            return Json2::Value();
        }
        jsonObject.Add(key.c_str(), value);
        return JsonReader::DepthCopy(jsonObject);
    } else {
        if (args.size() == 1) {
            return parseOneArg(args[0].c_str());
        } else {
            Json2::Value jsonArray = JsonReader::CreateArray();
            for (const auto &arg : args) {
                jsonArray.Add(parseOneArg(arg.c_str()));
            }
            return JsonReader::DepthCopy(jsonArray);
        }
    }
}

static void checkResponse(LocalSocket &localSocket, int &timeout)
{
    constexpr int defaultSleep = 10;
    string response;
    auto start = chrono::high_resolution_clock::now();
    while (chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count() <
        timeout) {
        localSocket >> response;
        if (response.length() != 0) {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(defaultSleep));
    }

    if (response.length()) {
        Json2::Value responseJson = JsonReader::ParseJsonData2(response);
        auto error = JsonReader::GetErrorPtr();
        if (error.length()) {
            fprintf(stderr, "Json error while parsing response: %s\n", error.c_str());
            printf("Response: %s\n", response.c_str());
        } else {
            printf("Response:\n%s\n", responseJson.ToStyledString().c_str());
        }
    } else {
        printf("No response provided.\n");
    }
}

int main(int argc, char *argv[])
{
    constexpr int defaultTimeout = 1000;
    panda::PandArgParser argparser;
    panda::PandArg<bool> helpArg("help", false, "Print this message and exit");
    panda::PandArg<string> nameArg("name", "", "Local socket name for command line interface.");
    panda::PandArg<int> timeoutArg("timeout", defaultTimeout, "Waiting response timeout in milliseconds.");
    argparser.Add(&helpArg);
    argparser.Add(&nameArg);
    argparser.EnableRemainder();
    if (!argparser.Parse(argc, argv) || helpArg.GetValue()) {
        printf("Usage:\n%s", argparser.GetHelpString().c_str());
        return 0;
    }
    Json2::Value result = parseArguments(argparser.GetRemainder());
    if (!result.IsValid()) {
        fprintf(stderr, "Invalid json args\n");
        return -1;
    }
    printf("Request:\n%s\n", result.ToStyledString().c_str());
    if (nameArg.GetValue().length() == 0) {
        fprintf(stderr, "Use --name <socket_name> to set socket name.\n");
        return -1;
    }
    LocalSocket localSocket;
    if (!localSocket.ConnectToServer(localSocket.GetCommandPipeName(nameArg.GetValue()),
        LocalSocket::OpenMode::READ_WRITE)) {
        fprintf(stderr, "Unable to connect to server socket \"%s\".\n",
            localSocket.GetCommandPipeName(nameArg.GetValue()).c_str());
        return -1;
    }
    if (result.IsString()) {
        localSocket << result.AsString();
    } else {
        localSocket << result.ToString();
    }
    int timeout = timeoutArg.GetValue();
    checkResponse(localSocket, timeout);
    return 0;
}
