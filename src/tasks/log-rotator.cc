//
//  log-rotator.cc
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

#include <iterator>
#include <unordered_set>
#include <vector>
#include <set>
#include <thread>
#include "src/logging/log.h"
#include "src/tasks/log-rotator.h"
#include "src/utils/utils.h"
#include "src/core/registry.h"
#include "src/crypto/zlib.h"
#include "src/core/residue-exception.h"

using namespace residue;

const unsigned long LogRotator::LENIENCY_THRESHOLD = 60 * 5; // 5 minutes

LogRotator::LogRotator(const std::string& name,
                       Registry* registry,
                       Configuration::RotationFrequency freq) :
    Task(name, registry, freq)
{
}

std::string LogRotator::checkStatus(const std::string& loggerId) const
{
    auto rotationFrequencies = m_registry->configuration()->rotationFreqencies();
    for (const auto& pair : rotationFrequencies) {
        if (loggerId != pair.first) {
            continue;
        }
        Configuration::RotationFrequency freq = pair.second;
        if (freq == Configuration::RotationFrequency::NEVER) {
            return "Not scheduled";
        }

        auto iter = m_lastRotation.find(loggerId);
        unsigned long lastRotated = iter == m_lastRotation.end() ? 0L : iter->second;
        std::stringstream ss;
        ss << name() << " scheduled to run ";
        if (lastRotated == 0L) {
            ss << "@ [" << formattedNextExecution() << "]";
        } else {
            unsigned long nextRotation = lastRotated + freq;
            bool skippedLast = false;
            if (Utils::now() > nextRotation) {
                // if next rotation is already past (just in case)
                // show the freq after this one
                nextRotation = nextRotation + freq;
                skippedLast = true;
            }
            ss << "@ " << Utils::formatTime(nextRotation, "%d %b, %Y %H:%m:%s");
            if (skippedLast) {
                ss << "\nLast rotation was skipped because of time inconsitency.";
            }
        }
        return ss.str();
    }
    return "Not scheduled";
}

void LogRotator::execute()
{
    // Check frequency for each logger with frequency
    auto rotationFrequencies = m_registry->configuration()->rotationFreqencies();
    for (const auto& pair : rotationFrequencies) {
        std::string loggerId = pair.first;
        Configuration::RotationFrequency freq = pair.second;

        if (freq == Configuration::RotationFrequency::NEVER) {
            return;
        }

        const unsigned long now = Utils::now();
#if 1
        if (now >= nextExecution()) {
            // should not need leniency...? ;/
            RLOG(INFO) << "Starting log rotation for logger [" << loggerId << "] {" << now << " >= " << nextExecution() << "}";
            rotate(loggerId);
            RLOG(INFO) << "Finished log rotation for logger [" << loggerId << "]";
        } else {
            RLOG(DEBUG) << "Ignoring rotation for [" << loggerId << "] - Reason: " << " now "
                       << now << ", schedule " << formattedNextExecution();
        }
#else

        // ---------- old

        auto iter = m_lastRotation.find(loggerId);
        unsigned long lastRotated = iter == m_lastRotation.end() ? 0L : iter->second;
        bool runRotation = (lastRotated == 0L && freq == Configuration::RotationFrequency::HOURLY)
                || now - lastRotated >= (freq - LENIENCY_THRESHOLD);
        if (runRotation) {
            RLOG(INFO) << "Rotating logs for logger [" << loggerId << "]";
            rotate(loggerId);
            m_lastRotation[loggerId] = Utils::now();
            RLOG(INFO) << "[" << loggerId << "] finished rotation at [" << m_lastRotation[loggerId] << "]";
        } else {
            RLOG(DEBUG) << "Ignoring rotation for [" << loggerId << "] - Reason: " << " lastRotated="
                       << lastRotated << ", delta: " << (now - lastRotated) << " < " << (freq - LENIENCY_THRESHOLD);
        }
#endif
    }

    archiveRotatedItems();
}

void LogRotator::archiveRotatedItems()
{
    RVLOG_IF(!m_archiveItems.empty(), RV_DETAILS) << "Archiving rotated logs... [Total loggers: " << m_archiveItems.size() << "]";
    for (auto& item : m_archiveItems) {
        std::thread t([&]() {
            el::Helpers::setThreadName(name() + "::LogArchiver");
            RLOG(INFO) << "Archiving for [" << item.loggerId << "] => [" << item.archiveFilename
                       << "] containing " << item.files.size() << " file(s)";
            archiveAndCompress(item.loggerId, item.archiveFilename, item.files);
        });
        t.join();
    }
    m_archiveItems.clear();
}


void LogRotator::rotate(const std::string& loggerId)
{
#ifdef RESIDUE_PROFILING
    unsigned long m_timeTaken;
    RESIDUE_PROFILE_START(t_rotation);
#endif
    time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    tm local_tm = *localtime(&tt);

    int currentMin = local_tm.tm_min;
    int currentHour = local_tm.tm_hour;
    int currentDay = local_tm.tm_mday;
    std::string currentWDay = el::base::consts::kDaysAbbrev[local_tm.tm_wday];
    std::string currentMonth = el::base::consts::kMonthsAbbrev[local_tm.tm_mon];
    std::string currentQuarter = std::to_string(std::ceil((local_tm.tm_mon + 1) / 3));
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

    std::string destinationDir = m_registry->configuration()->getArchivedLogDirectory(loggerId);
    std::string rotatedFilename = m_registry->configuration()->getArchivedLogFilename(loggerId);
    std::string archiveFilename = m_registry->configuration()->getArchivedLogCompressedFilename(loggerId);

    // For archiveFilename and rotatedFilename we do it twice for one may be in path
    // othe may be in the filename
    resolveFormatSpecifiers(destinationDir);
    resolveFormatSpecifiers(rotatedFilename);
    resolveFormatSpecifiers(rotatedFilename);
    resolveFormatSpecifiers(archiveFilename);
    resolveFormatSpecifiers(archiveFilename);

    destinationDir.append(el::base::consts::kFilePathSeperator);
    // Remove duplicate file separators
    Utils::replaceAll(destinationDir,
                      std::string(el::base::consts::kFilePathSeperator) +
                      std::string(el::base::consts::kFilePathSeperator),
                      el::base::consts::kFilePathSeperator);

    std::unordered_set<std::string> fileByLevel;
    std::map<std::string, std::set<std::string>> levelsInFilename;

    el::Logger* logger = el::Loggers::getLogger(loggerId);
    logger->acquireLock(); // NOTE: Be careful, do not log here using residue logger until releaseLock() below

    auto insertNow = [&](const std::string& filename, const std::string& levelIdentifier) {
        fileByLevel.insert(filename);
        std::string customIdentifier = logger->typedConfigurations()->filename(el::Level::Global) == filename
                ? "global" : levelIdentifier;
        if (levelsInFilename.find(filename) == levelsInFilename.end()) {
            levelsInFilename.insert(std::make_pair(filename, std::set<std::string> { customIdentifier }));
        } else {
            levelsInFilename[filename].insert(customIdentifier);
        }
    };

    std::string fnInfo = logger->typedConfigurations()->filename(el::Level::Info);
    std::string fnError = logger->typedConfigurations()->filename(el::Level::Error);
    std::string fnDebug = logger->typedConfigurations()->filename(el::Level::Debug);
    std::string fnWarning = logger->typedConfigurations()->filename(el::Level::Warning);
    std::string fnTrace = logger->typedConfigurations()->filename(el::Level::Trace);
    std::string fnVerbose = logger->typedConfigurations()->filename(el::Level::Verbose);
    std::string fnFatal = logger->typedConfigurations()->filename(el::Level::Fatal);

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

    insertNow(fnInfo, "info");
    insertNow(fnError, "error");
    insertNow(fnDebug, "debug");
    insertNow(fnWarning, "warning");
    insertNow(fnTrace, "trace");
    insertNow(fnVerbose, "verbose");
    insertNow(fnFatal, "fatal");

    // This set has three strings
    // { /tmp/log/muflihun.log, /tmp/log/verbose-muflihun.log, /tmp/log/debug-muflihun.log }
    // Our operation is target => destination map filenames by resolving format specifiers

    std::map<std::string, std::string> files;
    std::for_each(fileByLevel.begin(), fileByLevel.end(), [&](const std::string& sourceFilename) {
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
            RLOG_IF(loggerId != RESIDUE_LOGGER_ID, INFO) << "Prepending 'level' format specifier in filename for logger ["
                                                         << loggerId << "] as we have multiple filenames for levels";
            targetFilename = "%level-" + targetFilename;
        }

        Utils::replaceAll(destinationDir, "%original/", "%original"); // just a clean up

        Utils::replaceFirstWithEscape(destinationDir, "%level", levelName);
        Utils::replaceFirstWithEscape(targetFilename, "%level", levelName);
        Utils::replaceFirstWithEscape(destinationDir, "%original",
                                      el::base::utils::File::extractPathFromFilename(sourceFilename));

        if (!Utils::fileExists(destinationDir.c_str())) {
            if (!Utils::createPath(destinationDir.c_str())) {
                RLOG_IF(loggerId != RESIDUE_LOGGER_ID, ERROR) << "Failed to create path for log rotation: " << destinationDir;
                return;
            }
        }
        std::string fullDestinationPath = destinationDir + targetFilename;
        long fsize = Utils::fileSize(sourceFilename.c_str());
        if (fsize > 0) {
            RVLOG_IF(loggerId != RESIDUE_LOGGER_ID, RV_DETAILS) << "Rotating [" << sourceFilename
                                                                << "] => [" << fullDestinationPath
                                                                << "] (" << Utils::bytesToHumanReadable(fsize) << ")";
            if (::rename(sourceFilename.c_str(), fullDestinationPath.c_str()) == 0) {
                files.insert(std::make_pair(fullDestinationPath, targetFilename));
            } else {
                RLOG_IF(loggerId != RESIDUE_LOGGER_ID, ERROR) << "Error moving file ["
                                                              << sourceFilename << "] to ["
                                                              << fullDestinationPath << "] " << std::strerror(errno);
            }
        } else {
            RVLOG_IF(loggerId != RESIDUE_LOGGER_ID, RV_DETAILS) << "Ignoring rotating empty file " << sourceFilename;
        }
    });

    // Truncate log files

    el::base::type::fstream_t* fsInfo = logger->typedConfigurations()->fileStream(el::Level::Info);
    el::base::type::fstream_t* fsError = logger->typedConfigurations()->fileStream(el::Level::Error);
    el::base::type::fstream_t* fsDebug = logger->typedConfigurations()->fileStream(el::Level::Debug);
    el::base::type::fstream_t* fsWarning = logger->typedConfigurations()->fileStream(el::Level::Warning);
    el::base::type::fstream_t* fsTrace = logger->typedConfigurations()->fileStream(el::Level::Trace);
    el::base::type::fstream_t* fsVerbose = logger->typedConfigurations()->fileStream(el::Level::Verbose);
    el::base::type::fstream_t* fsFatal = logger->typedConfigurations()->fileStream(el::Level::Fatal);

    std::unordered_set<std::string> doneList;
    auto closeAndTrunc = [&](el::base::type::fstream_t* fs, const std::string& fn) {
        if (doneList.find(fn) != doneList.end()) {
            return;
        }
        fs->close();
        fs->open(fn, std::fstream::out | std::fstream::trunc);
        Utils::updateFilePermissions(fn.data(), logger, m_registry->configuration());
        doneList.insert(fn);
    };

    closeAndTrunc(fsInfo, fnInfo);
    closeAndTrunc(fsError, fnError);
    closeAndTrunc(fsWarning, fnWarning);
    closeAndTrunc(fsVerbose, fnVerbose);
    closeAndTrunc(fsFatal, fnFatal);
    closeAndTrunc(fsDebug, fnDebug);
    closeAndTrunc(fsTrace, fnTrace);

    logger->releaseLock();

#ifdef RESIDUE_PROFILING
    RESIDUE_PROFILE_END(t_rotation, m_timeTaken);
    float timeTakenInSec = static_cast<float>(m_timeTaken / 1000.0f);
    DRVLOG_IF(loggerId != RESIDUE_LOGGER_ID, RV_DEBUG) << "Took " << timeTakenInSec << " s rotate logs for logger [" << loggerId << "] (" << files.size() << " files)";
#endif
    m_archiveItems.push_back({loggerId, destinationDir + el::base::consts::kFilePathSeperator + archiveFilename, files});
}

void LogRotator::archiveAndCompress(const std::string& loggerId, const std::string& archiveFilename, const std::map<std::string, std::string>& files) {
    if (files.empty()) {
        return;
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

    bool compressSuccessful = false;

    try {
        compressSuccessful = ZLib::compressFile(archiveFilename, tmpTar);
    } catch (std::exception& e) {
        RLOG(ERROR) << "Exception while compressing; " << e.what();
    }

    const el::Logger* logger = el::Loggers::getLogger(loggerId, false);
    if (logger != nullptr) {
        RVLOG(RV_DETAILS) << "Updating permissions for " << archiveFilename << " against logger [" << loggerId << "]";
        Utils::updateFilePermissions(archiveFilename.data(), logger, m_registry->configuration());
    }

    if (compressSuccessful) {
        if (::remove(tmpTar.c_str()) != 0) {
            RLOG(WARNING) << "Compressed file successfully but failed to remove tar: " << std::strerror(errno);
        }
    } else {
        RLOG(ERROR) << "Failed to compress rotated log for logger [" << loggerId << "]. Destination name: [" << archiveFilename + "]";
    }
}

unsigned long LogRotator::calculateSecondsToMidnight(unsigned long now) const
{
    unsigned long minRoundOff = 3600 - (now % 3600);
    std::string next24HourStr = Utils::formatTime(minRoundOff + now, "%H");
    int next24Hour = atoi(next24HourStr.c_str());
    int hoursToNextIter = next24Hour == 0 || next24Hour == 12 ? 0 : 24 - next24Hour; // nearest day
    return minRoundOff + (hoursToNextIter * 3600);
}

unsigned long HourlyLogRotator::calculateRoundOff(unsigned long now) const
{
    return (3600 - (now % 3600)) - 1;
}

unsigned long SixHoursLogRotator::calculateRoundOff(unsigned long now) const
{
    unsigned long minRoundOff = 3600 - (now % 3600);

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

unsigned long TwelveHoursLogRotator::calculateRoundOff(unsigned long now) const
{
    unsigned long minRoundOff = 3600 - (now % 3600);

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

unsigned long DailyLogRotator::calculateRoundOff(unsigned long now) const
{
    return calculateSecondsToMidnight(now) - 1;
}

unsigned long WeeklyLogRotator::calculateRoundOff(unsigned long now) const
{

    const std::map<std::string, int> WEEK_DAYS_MAP = {
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

unsigned long MonthlyLogRotator::calculateRoundOff(unsigned long now) const
{
    const int year = atoi(Utils::formatTime(now, "%Y").c_str());
    const std::map<int, int> LAST_DAY_OF_MONTH_MAP = {
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

unsigned long YearlyLogRotator::calculateRoundOff(unsigned long now) const
{
    const int year = atoi(Utils::formatTime(now, "%Y").c_str());
    const std::map<int, int> LAST_DAY_OF_MONTH_MAP = {
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
