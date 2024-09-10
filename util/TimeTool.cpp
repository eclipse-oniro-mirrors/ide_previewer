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

#include "TimeTool.h"

#include <chrono>
#include <iostream>
#include <sstream>

#include "LocalDate.h"

const int BASE_YEAR = 1900;
const int MILLISECOND_PERSECOND = 1000;

std::string TimeTool::GetFormatTime()
{
    std::string timeNow = FormateTimeNow();
    std::string formatTime = "[" + timeNow + "]";
    return formatTime;
}

std::string TimeTool::GetTraceFormatTime()
{
    std::string traceTimeNow = FormateTimeNow();
    return traceTimeNow;
}

std::string TimeTool::FormateTimeNow()
{
    std::pair<tm, int64_t> timePair = GetCurrentTime();
    struct tm utcTime = timePair.first;
    int64_t msTime = timePair.second;
    const int fixedTimeWidth2 = 2;
    const int fixedTimeWidth3 = 3;
    const int fixedTimeWidth4 = 4;
    std::ostringstream now;
    now << FixedTime(utcTime.tm_year + BASE_YEAR, fixedTimeWidth4);
    now << "-";
    now << FixedTime(utcTime.tm_mon + 1, fixedTimeWidth2);
    now << "-";
    now << FixedTime(utcTime.tm_mday, fixedTimeWidth2);
    now << "T";
    now << FixedTime(utcTime.tm_hour, fixedTimeWidth2) ;
    now << ":";
    now << FixedTime(utcTime.tm_min, fixedTimeWidth2);
    now << ":";
    now << FixedTime(utcTime.tm_sec, fixedTimeWidth2);
    now << ".";
    now << FixedTime(msTime % MILLISECOND_PERSECOND, fixedTimeWidth3);
    return now.str();
}

std::string TimeTool::FixedTime(int32_t time, int32_t width)
{
    std::string tm = std::to_string(time);
    int len = tm.length();
    if (len < width) {
        for (int i = 0; i < width - len; i++) {
            tm = "0" + tm;
        }
    }
    return tm;
}

std::pair<tm, int64_t> TimeTool::GetCurrentTime()
{
    const std::time_t e8zone = 8 * 60 * 60 * 1000; // Time offset of GMT+08:00, in milliseconds, 8h*60m*60s*1000ms
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::milliseconds millsec = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::time_t ms = millsec.count() + e8zone;
    millsec = std::chrono::milliseconds(ms);
    now = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(millsec);
    auto time = std::chrono::system_clock::to_time_t(now);
    struct tm utcTime;
    LocalDate::GmTimeSafe(utcTime, time);
    std::pair<tm, int64_t> timePair = std::make_pair(utcTime, ms);
    return timePair;
}
