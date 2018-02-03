//
//  log-rotator-schedule-test.h
//  Residue
//
//  Copyright 2017-present Muflihun Labs
//
//  Author: @abumusamq
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#ifndef LOG_ROTATOR_SCHEDULE_TEST_H
#define LOG_ROTATOR_SCHEDULE_TEST_H

#include "test.h"
#include "src/utils/utils.h"
#include "src/tasks/log-rotator.h"

using namespace residue;

TEST(LogRotatorScheduleTest, HourlyRoundOffCalculation)
{
    TestData<int, int> TData = {
        { 1517579594, 406 }, // 00:53:14
        { 1517570594, 2206 }, //  22:23:14
    };

    for (auto& item : TData) {
        HourlyLogRotator logRotator(nullptr);

        ASSERT_EQ(item.get<1>(), logRotator.calculateRoundOff(item.get<0>()));
    }
}

TEST(LogRotatorScheduleTest, SixHoursRoundOffCalculation)
{
    TestData<int, int> TData = {
        { 1517570594, 5806  }, // Fri, 02/Feb/2018 22:23:14
        { 1517579594, 18406 }, //  Sat, 03/Feb/2018 00:53:14
        { 1517608394, 11206 }, //  Sat, 03/Feb/2018 08:53:14
        { 1517626394, 14806 }, //  Sat, 03/Feb/2018 13:53:14
    };

    for (auto& item : TData) {
        SixHoursLogRotator logRotator(nullptr);

        ASSERT_EQ(item.get<1>(), logRotator.calculateRoundOff(item.get<0>()));
    }
}

#endif // LOG_ROTATOR_SCHEDULE_TEST_H
