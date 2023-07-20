//
//  log-rotator.cc
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
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

#include "tasks/log-rotator.h"

#include <cmath>

#include <iterator>
#include <set>
#include <thread>
#include <vector>

#include "core/registry.h"
#include "core/residue-exception.h"
#include "crypto/zlib.h"
#include "extensions/pre-archive-extension.h"
#include "extensions/post-archive-extension.h"
#include "logging/log.h"
#include "utils/utils.h"

using namespace residue;

static const char* kDaysAbbrev[7]                   =      { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char* kMonthsAbbrev[12]                =      { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


LogRotator::LogRotator(const std::string& name,
                       Registry* registry,
                       Configuration::RotationFrequency freq) :
    Task(name, registry, freq),
    m_frequency(freq)
{
}

void LogRotator::execute()
{
    auto rotationFrequencies = m_registry->configuration()->rotationFreqencies();
    for (const auto& pair : rotationFrequencies) {
        if (m_frequency == pair.second) {
            std::string loggerId = pair.first;
            RLOG(INFO) << "Starting log rotation for logger [" << loggerId << "]";
            rotate(loggerId);
            RLOG(INFO) << "Finished log rotation for logger [" << loggerId << "]";
        }
    }

    archiveRotatedItems();
}

void LogRotator::archiveRotatedItems()
{
    RVLOG_IF(!m_archiveItems.empty(), RV_DETAILS) << "Archiving rotated logs... [Total loggers: " << m_archiveItems.size() << "]";
    for (auto& item : m_archiveItems) {
        std::thread t([&]() {
            el::Helpers::setThreadName(name() + "::LogArchiver");
            archiveAndCompress(item.loggerId, item.archiveFilename, item.files);
        });
        t.join();
    }
    m_archiveItems.clear();
}

LogRotator::RotateTarget LogRotator::createRotateTarget(const std::string& loggerId) const
{

    // we resolve all the format specifiers based on when the
    // task started instead of now, because loggers can take time
    // and may end up having a different values for format specifiers
    // for two loggers that were suppose to have same
    // e.g, logger A ran at 23:59:59 and took a while to archive
    // and then logger B ran at 00:00:00 so the %day will to resolve different
    // days for each loggers

    std::tm local_tm = Utils::timeToTm(lastExecution());

    DRVLOG_IF(loggerId != RESIDUE_LOGGER_ID, RV_DEBUG) << "FSR: logger [" << loggerId << "]";

    int currentMin = local_tm.tm_min;
    int currentHour = local_tm.tm_hour;
    int currentDay = local_tm.tm_mday;
    std::string currentWDay = kDaysAbbrev[local_tm.tm_wday];
    std::string currentMonth = kMonthsAbbrev[local_tm.tm_mon];
    std::string currentQuarter = "Q" + std::to_string(std::ceil((local_tm.tm_mon + 1) / 3));
    std::string currentYear = std::to_string(local_tm.tm_year + 1900);

    std::string currentHourStr = currentHour < 10 ?
                "0" + std::to_string(currentHour) : std::to_string(currentHour);
    std::string currentMinStr = currentMin < 10 ?
                "0" + std::to_string(currentMin) : std::to_string(currentMin);
    std::string currentDayStr = currentDay < 10 ?
                "0" + std::to_string(currentDay) : std::to_string(currentDay);

    auto resolveFormatSpecifiers = [&](std::string& templ) {
        Utils::replaceFirstWithEscape(templ, "%logger", loggerId);
        Utils::replaceFirstWithEscape(templ, "%min", currentMinStr);
        Utils::replaceFirstWithEscape(templ, "%hour", currentHourStr);
        Utils::replaceFirstWithEscape(templ, "%day", currentDayStr);
        Utils::replaceFirstWithEscape(templ, "%wday", currentWDay);
        Utils::replaceFirstWithEscape(templ, "%month", currentMonth);
        Utils::replaceFirstWithEscape(templ, "%quarter", currentQuarter);
        Utils::replaceFirstWithEscape(templ, "%year", currentYear);
    };

    std::string initDestinationDir = m_registry->configuration()->getArchivedLogDirectory(loggerId);
    std::string rotatedFilename = m_registry->configuration()->getArchivedLogFilename(loggerId);
    std::string archiveFilename = m_registry->configuration()->getArchivedLogCompressedFilename(loggerId);

    // For archiveFilename and rotatedFilename we do it twice for one may be in path
    // othe may be in the filename
    resolveFormatSpecifiers(initDestinationDir);
    resolveFormatSpecifiers(rotatedFilename);
    resolveFormatSpecifiers(rotatedFilename);
    resolveFormatSpecifiers(archiveFilename);
    resolveFormatSpecifiers(archiveFilename);

    initDestinationDir.append(el::base::consts::kFilePathSeparator);
    // Remove duplicate file separators
    Utils::replaceAll(initDestinationDir,
                      std::string(el::base::consts::kFilePathSeparator) +
                      std::string(el::base::consts::kFilePathSeparator),
                      el::base::consts::kFilePathSeparator);

    Utils::replaceAll(initDestinationDir, "%original/", "%original"); // just a clean up

    // up until here we have:
    //  * fully resolved archiveFilename
    //  * partially resolved initDestinationDir and rotatedFilename
    //      - rotatedFilename still needs to resolve %level
    //      - initDestinationDir still needs to resolve %level and %original
    std::vector<BackupItem> items;
    el::Logger* logger = el::Loggers::getLogger(loggerId, false);

    if (logger != nullptr) {

        //=========================== [ NOTE ]===============================
        //
        // Be careful, do not log here using residue logger until this scope
        //
        //===================================================================

        std::unordered_set<std::string> fileByLevel;

        std::unordered_map<std::string, std::set<std::string>> levelsInFilename;

        std::lock_guard<std::recursive_mutex> l(logger->lock());

        // mv fnInfo -> mylogs-17-00-19-Feb-info.log
        // mv fnError -> mylogs-17-00-19-Feb-error.log
        //
        // but if info, fatal and error are logging to /tmp/log/muflihun.log
        // but if warning and verbose are logging to /tmp/log/verbose-muflihun.log
        // and trace and debug is logging to /tmp/log/debug-muflihun.log
        //
        // we then:
        // mv fnInfo (consequently fnError and fnFatal) to mylogs-17-00-19-Feb-info-error.log
        // mv fnDebug (consequently fnTrace) to mylogs-17-00-19-Feb-debug.log
        // mv fnWarning (consequently fnVerbose) to mylogs-17-00-19-Feb-warning-verbose.log

        const std::string globalFilename = logger->typedConfigurations()->filename(el::Level::Global);

        el::base::type::EnumType lIndex = el::LevelHelper::kMinValid;
        el::LevelHelper::forEachLevel(&lIndex, [&](void) -> bool {
            el::Level level = el::LevelHelper::castFromInt(lIndex);
            std::string levelIdentifier(el::LevelHelper::convertToString(level));
            Utils::toLower(levelIdentifier);
            std::string filename = logger->typedConfigurations()->filename(level);
            DRVLOG_IF(loggerId != RESIDUE_LOGGER_ID, RV_DEBUG) << "FSR: level [" << levelIdentifier << "] => [" << filename << "]";

            fileByLevel.insert(filename);
            std::string customIdentifier = globalFilename == filename ? "global" : levelIdentifier;
            if (levelsInFilename.find(filename) == levelsInFilename.end()) {
                levelsInFilename.insert(std::make_pair(filename, std::set<std::string> { customIdentifier }));
            } else {
                levelsInFilename[filename].insert(customIdentifier);
            }
            return false;
        });

        items.reserve(fileByLevel.size());

        // This set has three strings
        // { /tmp/log/muflihun.log, /tmp/log/verbose-muflihun.log, /tmp/log/debug-muflihun.log }
        // Our operation is target => destination map filenames by resolving format specifiers

        for (const auto& sourceFilename : fileByLevel) {

            // we need copy to preserve %level for other levels
            std::string targetFilename = rotatedFilename;

            const std::set<std::string>& levels = levelsInFilename.at(sourceFilename);
            std::stringstream ss;
            std::transform(levels.begin(), levels.end(),
                           std::ostream_iterator<std::string>(ss, "-"), [](const std::string& s) {
                return s;
            });
            std::string levelName(ss.str());
            levelName.erase(levelName.end() - 1); // Remove last hyphen

            if (fileByLevel.size() > 1 && targetFilename.find("%level") == std::string::npos) {
                // we have multiple files for each level but no level specifier provided in target filename,
                // just prepend it
                RLOG_IF(loggerId != RESIDUE_LOGGER_ID, INFO) << "FSR: Prepending 'level' format specifier in filename for logger ["
                                                             << loggerId << "] as we have multiple filenames for levels";
                targetFilename = "%level-" + targetFilename;
            }

            Utils::replaceFirstWithEscape(initDestinationDir, "%original",
                                          el::base::utils::File::extractPathFromFilename(sourceFilename));

            std::string destinationDir = initDestinationDir;

            Utils::replaceFirstWithEscape(destinationDir, "%level", levelName);
            Utils::replaceFirstWithEscape(targetFilename, "%level", levelName);

            RLOG_IF(loggerId != RESIDUE_LOGGER_ID, INFO) << "FSR: [" << sourceFilename << "] => [" << destinationDir << "] as [" << targetFilename << "]";

            items.push_back({ sourceFilename, destinationDir, targetFilename });
        }
    }

    Utils::replaceAll(initDestinationDir, "%level", ""); // absolutely cannot have levels

    RLOG_IF(loggerId != RESIDUE_LOGGER_ID, INFO) << "FSR: Result: [" << initDestinationDir << "] [" << archiveFilename << "] with [" << items.size() << "] items";

    return { initDestinationDir, archiveFilename, items };
}

void LogRotator::rotate(const std::string& loggerId)
{
#ifdef RESIDUE_PROFILING
    types::Time m_timeTaken;
    RESIDUE_PROFILE_START(t_rotation);
#endif

    const RotateTarget rotateTarget = createRotateTarget(loggerId);

    std::unordered_map<std::string, std::string> files;
    el::Logger* logger = el::Loggers::getLogger(loggerId, false);

    if (logger != nullptr) {

        //=========================== [ NOTE ]===============================
        //
        // Be careful, do not log here using residue logger until this scope
        //
        //===================================================================

        std::lock_guard<std::recursive_mutex> l(logger->lock());

        // Create backups

        for (const auto& backItem : rotateTarget.items) {

            if (!Utils::fileExists(backItem.destinationDir.c_str())) {
                if (!Utils::createPath(backItem.destinationDir.c_str())) {
                    RLOG_IF(loggerId != RESIDUE_LOGGER_ID, ERROR) << "Failed to create path for log rotation: " << backItem.destinationDir;
                    return;
                }
            }
            std::string fullDestinationPath = backItem.destinationDir + backItem.targetFilename;
            long fsize = Utils::fileSize(backItem.sourceFilename.c_str());
            if (fsize > 0) {
                RVLOG_IF(loggerId != RESIDUE_LOGGER_ID, RV_DETAILS) << "Rotating [" << backItem.sourceFilename
                                                                    << "] => [" << fullDestinationPath
                                                                    << "] (" << Utils::bytesToHumanReadable(fsize) << ")";
                if (::rename(backItem.sourceFilename.c_str(), fullDestinationPath.c_str()) == 0) {
                    files.insert(std::make_pair(fullDestinationPath, backItem.targetFilename));
                } else {
                    RLOG_IF(loggerId != RESIDUE_LOGGER_ID, ERROR) << "Error moving file ["
                                                                  << backItem.sourceFilename << "] to ["
                                                                  << fullDestinationPath << "] " << std::strerror(errno);
                }
            } else {
                RVLOG_IF(loggerId != RESIDUE_LOGGER_ID, RV_DETAILS) << "Ignoring rotating empty file " << backItem.sourceFilename;
            }
        }

        // Truncate log files

        std::unordered_set<std::string> doneList;
        auto closeAndTrunc = [&](el::base::type::fstream_t* fs, const std::string& fn) {
            if (doneList.find(fn) != doneList.end()) {
                return;
            }
            if (fs && fs->is_open() && !fs->fail()) {
                fs->close();
                fs->open(fn, std::fstream::out | std::fstream::trunc);
                Utils::updateFilePermissions(fn.data(), logger, m_registry->configuration());
            }
            doneList.insert(fn);
        };


        el::base::type::EnumType lIndex = el::LevelHelper::kMinValid;
        el::LevelHelper::forEachLevel(&lIndex, [&](void) -> bool {
            el::Level level = el::LevelHelper::castFromInt(lIndex);
            closeAndTrunc(logger->typedConfigurations()->fileStream(level), logger->typedConfigurations()->filename(level));
            return false;
        });

    } // scope for logger lock

#ifdef RESIDUE_PROFILING
    RESIDUE_PROFILE_END(t_rotation, m_timeTaken);
    float timeTakenInSec = static_cast<float>(m_timeTaken / 1000.0f);
    DRVLOG_IF(loggerId != RESIDUE_LOGGER_ID, RV_DEBUG) << "Took " << timeTakenInSec << " s rotate logs for logger [" << loggerId << "] (" << files.size() << " files)";
#endif
    m_archiveItems.push_back({loggerId, rotateTarget.destinationDir + el::base::consts::kFilePathSeparator + rotateTarget.archiveFilename, files});
}

void LogRotator::archiveAndCompress(const std::string& loggerId, const std::string& archiveFilename, const std::unordered_map<std::string, std::string>& files) {
    if (files.empty()) {
        RLOG(INFO) << "No file to archive for [" << loggerId << "]";
        return;
    }

    RLOG(INFO) << "Archiving for [" << loggerId << "] => [" << archiveFilename
               << "] containing " << files.size() << " file(s)";


    if (!m_registry->configuration()->preArchiveExtensions().empty()) {
        PreArchiveExtension::Data d {
            loggerId,
            archiveFilename,
            files
        };
        bool continueProcess = true;
        for (auto& ext : m_registry->configuration()->preArchiveExtensions()) {
            auto extResult = ext->trigger(&d);
            continueProcess = continueProcess && extResult.continueProcess;
        }

        if (!continueProcess) {
            RLOG(INFO) << "Stopping archive process for logger [" << loggerId << "] because of one of the [pre_archive] extensions";
            return;
        }
    }

    // compress files after logger's lock is released
    RVLOG(RV_DETAILS) << "Compressing rotated files for logger [" << loggerId << "] to [" << archiveFilename << "]";

    std::string tmpTar = archiveFilename + ".tar";
    if (!Utils::archiveFiles(tmpTar, files)) {
        RLOG(ERROR) << "Failed to archive rotated log for logger [" << loggerId << "]. Destination name: [" << archiveFilename + ".tar" << "]";
        return;
    }

    for (auto& f : files) {
        RVLOG(RV_DETAILS) << "Removing plain file [" << f.first << "] (" << Utils::bytesToHumanReadable(Utils::fileSize(f.first.c_str())) << ")";
        if (::remove(f.first.c_str()) != 0) {
            RLOG(ERROR) << "Error removing file [" << f.first << "] " << std::strerror(errno);
        }
    }

    bool compressSuccessful = ZLib::compressFile(archiveFilename, tmpTar); // nothrow func

    const el::Logger* logger = el::Loggers::getLogger(loggerId, false);
    if (logger != nullptr) {
        RVLOG(RV_DETAILS) << "Updating permissions for " << archiveFilename << " against logger [" << loggerId << "]";
        Utils::updateFilePermissions(archiveFilename.data(), logger, m_registry->configuration());
    }

    if (compressSuccessful) {
        if (::remove(tmpTar.c_str()) != 0) {
            RLOG(WARNING) << "Compressed file successfully but failed to remove tar: " << std::strerror(errno);
        }

        if (!m_registry->configuration()->postArchiveExtensions().empty()) {
            PostArchiveExtension::Data d {
                loggerId,
                archiveFilename
            };
            for (auto& ext : m_registry->configuration()->postArchiveExtensions()) {
                ext->trigger(&d);
            }
        }

    } else {
        RLOG(ERROR) << "Failed to compress rotated log for logger [" << loggerId << "]. Destination name: [" << archiveFilename + "]";
    }
}

types::Time LogRotator::calculateSecondsToMidnight(types::Time now) const
{
    types::Time minRoundOff = 3600 - (now % 3600);
    std::string next24HourStr = Utils::formatTime(minRoundOff + now, "%H");
    int next24Hour = atoi(next24HourStr.c_str());
    int hoursToNextIter = next24Hour == 0 ? 0 : 24 - next24Hour; // nearest day
    return minRoundOff + (hoursToNextIter * 3600);
}

types::Time HourlyLogRotator::calculateRoundOff(types::Time now) const
{
    return (3600 - (now % 3600)) - 1;
}

types::Time SixHoursLogRotator::calculateRoundOff(types::Time now) const
{
    types::Time minRoundOff = 3600 - (now % 3600);

    std::string nextHourStr = Utils::formatTime(minRoundOff + now, "%h");
    int nextHour = atoi(nextHourStr.c_str());

    int hoursToNextIter = 12 - nextHour;
    if (hoursToNextIter > 6) {
        hoursToNextIter -= 6;
    }
    if (nextHour == 0 || nextHour == 12) {
        hoursToNextIter = 0;
    }
    return minRoundOff + (hoursToNextIter * 3600);
}

types::Time TwelveHoursLogRotator::calculateRoundOff(types::Time now) const
{
    types::Time minRoundOff = 3600 - (now % 3600);

    std::string next24HourStr = Utils::formatTime(minRoundOff + now, "%H");
    int next24Hour = atoi(next24HourStr.c_str());

    int hoursToNextIter = 24 - next24Hour;
    if (hoursToNextIter > 12) {
        hoursToNextIter -= 12;
    }

    if (next24Hour == 0 || next24Hour == 12) {
        hoursToNextIter = 0;
    }
    return minRoundOff + (hoursToNextIter * 3600);
}

types::Time DailyLogRotator::calculateRoundOff(types::Time now) const
{
    return calculateSecondsToMidnight(now) - 1;
}

types::Time WeeklyLogRotator::calculateRoundOff(types::Time now) const
{

    const std::unordered_map<std::string, int> WEEK_DAYS_MAP = {
        { "Mon", 1 },
        { "Tue", 2 },
        { "Wed", 3 },
        { "Thu", 4 },
        { "Fri", 5 },
        { "Sat", 6 },
        { "Sun", 7 }
    };

    // Section-1
    int secsToNextMidnight = calculateSecondsToMidnight(now);

    // Section-2
    std::string weekDayStr = Utils::formatTime(now + secsToNextMidnight, "%a");
    int weekDay = WEEK_DAYS_MAP.at(weekDayStr);

    int daysToNextMonday = weekDay == 1 ? 0 : (7 - weekDay) + 1;

    DRVLOG(RV_DEBUG) << "now: " << Utils::formatTime(now, "%a, %d/%M/%Y %h:%m:%s")
                     << " Weekday: " << weekDayStr
                     << " daysToNextMonday " << daysToNextMonday;

    return (secsToNextMidnight + (daysToNextMonday * 86400)) - 1;
}

types::Time MonthlyLogRotator::calculateRoundOff(types::Time now) const
{
    const int year = atoi(Utils::formatTime(now, "%Y").c_str());
    const std::unordered_map<int, int> LAST_DAY_OF_MONTH_MAP = {
        { 1, 31 },
        { 2, year % 4 == 0 ? 29 : 28 },
        { 3, 31 },
        { 4, 30 },
        { 5, 31 },
        { 6, 30 },
        { 7, 31 },
        { 8, 31 },
        { 9, 30 },
        { 10, 31 },
        { 11, 30 },
        { 12, 31 }
    };

    // Section-1
    int secsToNextMidnight = calculateSecondsToMidnight(now);

    // Section-2
    std::string monthStr = Utils::formatTime(secsToNextMidnight + now, "%M");
    std::string monthDayStr = Utils::formatTime(secsToNextMidnight + now, "%d");
    int month = atoi(monthStr.c_str());
    int monthDay = atoi(monthDayStr.c_str());
    int lastDayOfThisMonth = LAST_DAY_OF_MONTH_MAP.at(month);
    int daysToNextMonth = monthDay == 1 ? 0 : (lastDayOfThisMonth - monthDay) + 1;

    DRVLOG(RV_DEBUG) << " now " << Utils::formatTime(now, "%a, %d/%M/%Y %h:%m:%s")
                     << " secsToNextMidnight " << secsToNextMidnight
                     << " daysToNextMonth " << daysToNextMonth
                     << " monthDay " << monthDay
                     << " monthDayStr " << monthDayStr
                     << " lastDayOfThisMonth " << lastDayOfThisMonth;

    return (secsToNextMidnight + (daysToNextMonth * 86400)) - 1;
}

types::Time YearlyLogRotator::calculateRoundOff(types::Time now) const
{
    const int year = atoi(Utils::formatTime(now, "%Y").c_str());
    const std::unordered_map<int, int> LAST_DAY_OF_MONTH_MAP = {
        { 1, 31 },
        { 2, year % 4 == 0 ? 29 : 28 },
        { 3, 31 },
        { 4, 30 },
        { 5, 31 },
        { 6, 30 },
        { 7, 31 },
        { 8, 31 },
        { 9, 30 },
        { 10, 31 },
        { 11, 30 },
        { 12, 31 }
    };

    // Section-1
    int secsToNextMidnight = calculateSecondsToMidnight(now);

    // Section-2
    std::string monthStr = Utils::formatTime(secsToNextMidnight + now, "%M");
    std::string monthDayStr = Utils::formatTime(secsToNextMidnight + now, "%d");
    int month = atoi(monthStr.c_str());
    int monthDay = atoi(monthDayStr.c_str());
    int lastDayOfThisMonth = LAST_DAY_OF_MONTH_MAP.at(month);
    int daysToNextMonth = monthDay == 1 ? 0 : (lastDayOfThisMonth - monthDay) + 1;

    // Section-3
    int monthsToNextYear = (12 - month) + (daysToNextMonth == 0 ? 1 : 0);
    if (monthsToNextYear == 12) {
        monthsToNextYear = 0;
    }
    int extraDays = 0;
    for (int i = monthsToNextYear - 1; i >= 0 && monthsToNextYear > 0; --i) {
        extraDays += LAST_DAY_OF_MONTH_MAP.at(12 - i) - 28;
    }

    DRVLOG(RV_DEBUG) << " now " << Utils::formatTime(now, "%a, %d/%M/%Y %h:%m:%s")
                     << " month: " << month
                     << " secsToNextMidnight " << secsToNextMidnight
                     << " daysToNextMonth: " << daysToNextMonth
                     << " monthsToNextYear: " << monthsToNextYear
                     << " extraDays: " << extraDays;

    return (secsToNextMidnight + (daysToNextMonth * 86400) + (monthsToNextYear * 28 * 86400) + (extraDays * 86400)) - 1;
}
