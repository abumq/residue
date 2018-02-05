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

#include <map>
#include <memory>
#include "test.h"
#include "src/utils/utils.h"
#include "src/tasks/log-rotator.h"

using namespace residue;

static unsigned long s_offset = offsetTimezone * 3600;
static unsigned long s_soffset = offsetTimezone == 0 ? 0 : 1 * 3600;

void displayFormattedResult(const LogRotator* logRotator,
                            const TestCase<unsigned long, unsigned long, std::string>& item)
{
    unsigned long newEpoch = item.get<0>() + logRotator->calculateRoundOff(item.get<0>());
    const char* format = "%a, %d/%b/%Y %H:%m:%s";
    std::string formatted = Utils::formatTime(newEpoch, format);
    std::string orig = Utils::formatTime(item.get<0>(), format);
    std::cout << orig << " (" << item.get<0>() << ")" << " ==> " << formatted << " (" << newEpoch << ")" << std::endl;
}

TEST(LogRotatorScheduleTest, HourlyRoundOffCalculation)
{
    TestData<unsigned long, unsigned long, std::string> TData = {
        { 1517579594, 406, "Fri, 02/Feb/2018 00:53:14" },
        { 1517570594, 2206, "Fri, 02/Feb/2018 22:23:14" },
        { 1517608394, 406, "Sat, 03/Feb/2018 08:53:14" },
        { 1517626395, 405, "Sat, 03/Feb/2018 13:53:14" },
    };

    for (auto& item : TData) {
        HourlyLogRotator logRotator(nullptr);

        ASSERT_EQ(item.get<1>() - 1, logRotator.calculateRoundOff(item.get<0>())) << item.get<2>();

        displayFormattedResult(&logRotator, item);
    }
}

TEST(LogRotatorScheduleTest, SixHoursRoundOffCalculation)
{
    TestData<unsigned long, unsigned long, std::string> TData = {
        { 1517570594, 5806, "Fri, 02/Feb/2018 22:23:14" },
        { 1517579594, 18406, "Sat, 03/Feb/2018 00:53:14" },
        { 1517608394, 11206, "Sat, 03/Feb/2018 08:53:14" },
        { 1517626394, 14806, "Sat, 03/Feb/2018 13:53:14" },
    };

    for (auto& item : TData) {
        SixHoursLogRotator logRotator(nullptr);

        ASSERT_EQ(item.get<1>() - s_soffset, logRotator.calculateRoundOff(item.get<0>())) << item.get<2>();

        displayFormattedResult(&logRotator, item);
    }
}

TEST(LogRotatorScheduleTest, TwelveHoursRoundOffCalculation)
{
    TestData<unsigned long, unsigned long, std::string> TData = {
        { 1517570594, 5806, "Fri, 02/Feb/2018 22:23:14" },
        { 1517579594, 40006, "Sat, 03/Feb/2018 00:53:14" },
        { 1517608394, 11206, "Sat, 03/Feb/2018 08:53:14" },
        { 1517626394, 36406, "Sat, 03/Feb/2018 13:53:14" },
    };

    for (auto& item : TData) {
        TwelveHoursLogRotator logRotator(nullptr);

        ASSERT_EQ((item.get<1>()) + offsetTimezone, logRotator.calculateRoundOff(item.get<0>())) << item.get<2>();

        displayFormattedResult(&logRotator, item);
    }
}

TEST(LogRotatorScheduleTest, DailyRoundOffCalculation)
{
    TestData<unsigned long, unsigned long, std::string> TData = {
        { 1517570594, 5806, "Fri, 02/Feb/2018 22:23:14" },
        { 1517579594, 83206, "Sat, 03/Feb/2018 00:53:14" },
        { 1517608394, 54406, "Sat, 03/Feb/2018 08:53:14" },
        { 1517626394, 36406, "Sat, 03/Feb/2018 13:53:14" },
    };

    for (auto& item : TData) {
        DailyLogRotator logRotator(nullptr);

        ASSERT_EQ((item.get<1>() - 1) + offsetTimezone, logRotator.calculateRoundOff(item.get<0>())) << item.get<2>();

        displayFormattedResult(&logRotator, item);
    }
}

TEST(LogRotatorScheduleTest, WeeklyRoundOffCalculation)
{
    TestData<unsigned long, unsigned long, std::string> TData = {
        //             days in
        //            until next
        //              monday
        { 1517570594, (2*86400) + 5806, "Fri, 02/Feb/2018 22:23:14" },
        { 1517635888, (1*86400) + 26912, "Sat, 03/Feb/2018 16:31:28" },
        { 1517746897, (0*86400) + 2303, "Sun, 04/Feb/2018 23:21:37" },
        { 1517833297, (6*86400) + 2303, "Mon, 05/Feb/2018 23:21:37" },
        { 1517919697, (5*86400) + 2303, "Tue, 06/Feb/2018 23:21:37" },
        { 1518006097, (4*86400) + 2303, "Wed, 07/Feb/2018 23:21:37" },
        { 1518092497, (3*86400) + 2303, "Thu, 08/Feb/2018 23:21:37" },
    };

    for (auto& item : TData) {
        WeeklyLogRotator logRotator(nullptr);

        ASSERT_EQ((item.get<1>() - 1) + offsetTimezone, logRotator.calculateRoundOff(item.get<0>())) << item.get<2>();

        displayFormattedResult(&logRotator, item);
    }
}

TEST(LogRotatorScheduleTest, MonthlyRoundOffCalculation)
{
    TestData<unsigned long, unsigned long, std::string> TData = {
        //             days in
        //            next month
        { 1517570594, (26*86400) + 5806, "Fri, 02/Feb/2018 22:23:14"  },
        { 1519734097, (1*86400) + 2303, "Tue, 27/Feb/2018 23:21:37" },
        { 1519820497, (0*86400) + 2303, "Wed, 28/Feb/2018 23:21:37" },
        { 1582892497, (1*86400) + 2303, "Fri, 28/Feb/2020 23:21:37 - leap year" },
        { 1520252497, (26*86400) + 2303, "Mon, 05/Mar/2018 23:21:37" },
        { 1522412497, (1*86400) + 2303, "Fri, 30/Mar/2018 23:21:37" },
        { 1522326097, (2*86400) + 2303, "Thu, 29/Mar/2018 23:21:37" },
        { 1522498897, (0*86400) + 2303, "Sat, 31/Mar/2018 23:21:37" },
        { 1525094497, (0*86400) + 2303, "Mon, 30/Apr/2018 23:21:37 "},
        { 1525008097, (1*86400) + 2303, "Sun, 29/Apr/2018 23:21:37 "},
        { 1519906897, (30*86400) + 2303, "Thu, 01/Mar/2018 23:21:37" },
        { 1530451297, (30*86400) + 2303, "Sun, 01/Jul/2018 23:21:37" },
        { 1532956897, (1*86400) + 2303, "Mon, 30/Jul/2018 23:21:37" },
        { 1533043297, (0*86400) + 2303, "Tue, 31/Jul/2018 23:21:37" },
        { 1546258897, (0*86400) + 2303, "Mon, 31/Dec/2018 23:21:37" },
        { 1546345297, (30*86400) + 2303, "Tue, 01/Jan/2019 23:21:37" },
    };

    for (auto& item : TData) {
        MonthlyLogRotator logRotator(nullptr);

        ASSERT_EQ((item.get<1>() - 1) + offsetTimezone, logRotator.calculateRoundOff(item.get<0>())) << item.get<2>();

        displayFormattedResult(&logRotator, item);
    }
}

TEST(LogRotatorScheduleTest, YearlyRoundOffCalculation)
{
    TestData<unsigned long, unsigned long, std::string> TData = {
        //             days in       months to    extra days
        //            next month     next year
        { 1517570594, (26*86400) + (10*2419200) + (26 * 86400) + 5806, "Fri, 02/Feb/2018 22:23:14"  },
        { 1519734097, (1*86400) + (10*2419200) + (26 * 86400) + 2303, "Tue, 27/Feb/2018 23:21:37" },
        { 1519820497, (0*86400) + (10*2419200) + (26 * 86400) + 2303, "Wed, 28/Feb/2018 23:21:37" },
        { 1582892497, (1*86400) + (10*2419200) + (26 * 86400) + 2303, "Fri, 28/Feb/2020 23:21:37 - leap year" },
        { 1579090897, (16*86400) + (11*2419200) + (27 /* 26+1 because of leap year */ * 86400) + 2303, "Wed, 15/Jan/2020 23:21:37 - leap year" },
        { 1520252497, (26*86400) + (9*2419200) + (23 * 86400) + 2303, "Mon, 05/Mar/2018 23:21:37" },
        { 1522412497, (1*86400) + (9*2419200) + (23 * 86400) + 2303, "Fri, 30/Mar/2018 23:21:37" },
        { 1522326097, (2*86400) + (9*2419200) + (23 * 86400) + 2303, "Thu, 29/Mar/2018 23:21:37" },
        { 1522498897, (0*86400) + (9*2419200) + (23 * 86400) + 2303, "Sat, 31/Mar/2018 23:21:37" },
        { 1519906897, (30*86400) + (9*2419200) + (23 * 86400) + 2303, "Thu, 01/Mar/2018 23:21:37" },

        // with dst
        { 1525094497, (0*86400) + (8*2419200) + (21 * 86400) + 2303, "Mon, 30/Apr/2018 23:21:37 "},
        { 1525008097, (1*86400) + (8*2419200) + (21 * 86400) + 2303, "Sun, 29/Apr/2018 23:21:37 "},
        { 1530451297, (30*86400) + (5*2419200) + (13 * 86400) + 2303, "Sun, 01/Jul/2018 23:21:37" },
        { 1532956897, (1*86400) + (5*2419200) + (13 * 86400) + 2303, "Mon, 30/Jul/2018 23:21:37" },
        { 1533043297, (0*86400) + (5*2419200) + (13 * 86400) + 2303, "Tue, 31/Jul/2018 23:21:37" },
        { 1535721697, (0*86400) + (4*2419200) + (10 * 86400) + 2303, "Fri, 31/Aug/2018 23:21:37" },
        { 1538313697, (0*86400) + (3*2419200) + (8 * 86400) + 2303, "Sun, 30/Sep/2018 23:21:37" },

        { 1540988497, (0*86400) + (2*2419200) + (5 * 86400) + 2303, "Wed, 31/Oct/2018 23:21:37" },
        { 1546345297, (30*86400) + (11*2419200) + (26 * 86400) + 2303, "Tue, 01/Jan/2019 23:21:37" },
        { 1546258897, (0*86400) + (0*2419200) + (0 * 86400) + 2303, "Mon, 31/Dec/2018 23:21:37" },
        { 1609417297, (0*86400) + (0*2419200) + (0 * 86400) + 2303, "Mon, 31/Dec/2020 23:21:37 - leap year" },
        { 1577794897, (0*86400) + (0*2419200) + (0 * 86400) + 2303, "Mon, 31/Dec/2019 23:21:37" },
    };

    for (auto& item : TData) {
        YearlyLogRotator logRotator(nullptr);

        ASSERT_EQ((item.get<1>() - 1) + offsetTimezone, logRotator.calculateRoundOff(item.get<0>())) << item.get<2>();

        displayFormattedResult(&logRotator, item);

        unsigned long newEpoch = item.get<0>() + logRotator.calculateRoundOff(item.get<0>());
        int year = atoi(Utils::formatTime(item.get<0>(), "%Y").c_str());
        int month = atoi(Utils::formatTime(item.get<0>(), "%M").c_str());
        bool dst = month >= 4 && month <= 9;
        switch (year) {
        case 2018:
            ASSERT_EQ(newEpoch, 1546261199 + (dst ? 3600 : 0));
            break;
        case 2019:
            ASSERT_EQ(newEpoch, 1577797199 + (dst ? 3600 : 0));
            break;
        case 2020:
            ASSERT_EQ(newEpoch, 1609419599 + (dst ? 3600 : 0));
            break;
        }
    }
}

#endif // LOG_ROTATOR_SCHEDULE_TEST_H
