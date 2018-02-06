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

#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>
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
    struct ArchiveItem
    {
        std::string loggerId;
        std::string archiveFilename;
        std::map<std::string, std::string> files;
    };

    struct ArchiveFilenames
    {
        std::string destinationDir;
        std::string rotatedFilename;
        std::string archiveFilename;
    };

    LogRotator(const std::string& name,
               Registry* registry,
               Configuration::RotationFrequency freq);
    virtual ~LogRotator() = default;

    void rotate(const std::string& loggerId);
    void archiveRotatedItems();

    ///
    /// \brief This function builds archive filenames partially. It does not resolve
    /// for %level (and %original as they can change by level)
    ///
    ArchiveFilenames resolveInitialFormatSpecifiers(const std::string& loggerId) const;

    inline Configuration::RotationFrequency frequency() const
    {
        return m_frequency;
    }
protected:
    virtual void execute() override;

    types::Time calculateSecondsToMidnight(types::Time now) const;
private:
    std::vector<ArchiveItem> m_archiveItems;
    Configuration::RotationFrequency m_frequency;

    void archiveAndCompress(const std::string&,
                            const std::string&,
                            const std::map<std::string, std::string>&);
};

#define DECL_LOG_ROTATOR(ID, NAME, FREQ)\
class NAME : public LogRotator\
{\
public:\
    explicit NAME(Registry* registry) \
        : LogRotator(ID, registry, Configuration::RotationFrequency::FREQ) \
    {}\
    \
    virtual types::Time calculateRoundOff(types::Time now) const override;\
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
