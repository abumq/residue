//
//  log-rotator.h
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

#ifndef LogRotator_h
#define LogRotator_h

#include <unordered_map>
#include <map>
#include <vector>
#include "src/core/configuration.h"
#include "src/tasks/task.h"

namespace residue {

class Registry;

///
/// \brief Log rotator task
///
class LogRotator : public Task
{
public:
    virtual ~LogRotator() = default;
    ///
    /// \brief If log rotator is running and frequency is this threshold (in seconds) away,
    /// it will run it anyway
    ///
    static const unsigned long LENIENCY_THRESHOLD;

    struct ArchiveItem {
        std::string loggerId;
        std::string archiveFilename;
        std::map<std::string, std::string> files;
    };

    explicit LogRotator(const std::string& name,
                        Registry* registry,
                        Configuration::RotationFrequency freq);
    void rotate(const std::string& loggerId);
    void archiveRotatedItems();
    std::string checkStatus(const std::string& loggerId) const;
protected:
    virtual void execute() override;

    unsigned long calculateSecondsToMidnight(unsigned long now) const;
private:
    std::vector<ArchiveItem> m_archiveItems;
    std::unordered_map<std::string, unsigned long> m_lastRotation;

    void archiveAndCompress(const std::string&,
                            const std::string&,
                            const std::map<std::string, std::string>&);
};

#define DECL_LOG_ROTATOR(ID, NAME, FREQ)\
class NAME final : public LogRotator\
{\
public:\
    explicit NAME(Registry* registry) \
        : LogRotator(ID, registry, Configuration::RotationFrequency::FREQ) \
    {}\
    \
    virtual unsigned long calculateRoundOff(unsigned long now) const override;\
}

DECL_LOG_ROTATOR("HourlyLogRotator", HourlyLogRotator, HOURLY);
DECL_LOG_ROTATOR("SixHoursLogRotator", SixHoursLogRotator, SIX_HOURS);
DECL_LOG_ROTATOR("TwelveHoursLogRotator", TwelveHoursLogRotator, TWELVE_HOURS);
DECL_LOG_ROTATOR("DailyLogRotator", DailyLogRotator, DAILY);
DECL_LOG_ROTATOR("WeeklyLogRotator", WeeklyLogRotator, WEEKLY);
DECL_LOG_ROTATOR("MonthlyLogRotator", MonthlyLogRotator, MONTHLY);
DECL_LOG_ROTATOR("YearlyLogRotator", YearlyLogRotator, YEARLY);

#undef DECL_LOG_ROTATOR

}
#endif /* LogRotator_h */
