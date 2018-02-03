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

#include <ctime>
#include <cstdio>
#include <iterator>
#include <unordered_set>
#include <vector>
#include <set>
#include <thread>
#include "include/log.h"
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

std::string LogRotator::checkStatus(const std::string& loggerId)
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
        ss << "Scheduled to run ";
        if (lastRotated == 0L) {
            ss << " @ [" << formattedNextExecution() << "]";
        } else {
            unsigned long nextRotation = lastRotated + freq;
            bool skippedLast = false;
            if (Utils::now() > nextRotation) {
                // if next rotation is already past (just in case)
                // show the freq after this one
                nextRotation = nextRotation + freq;
                skippedLast = true;
            }
            ss << "at " << Utils::formatTime(nextRotation, "%d %b, %Y %H:%m:%s");
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

        const unsigned long now = Utils::now();
        auto iter = m_lastRotation.find(loggerId);
        unsigned long lastRotated = iter == m_lastRotation.end() ? 0L : iter->second;
        bool runRotation = freq != Configuration::RotationFrequency::NEVER
                && ((lastRotated == 0L && freq == Configuration::RotationFrequency::HOURLY)
                    || now - lastRotated >= (freq - LENIENCY_THRESHOLD));
        if (runRotation) {
            RLOG(INFO) << "Rotating logs for logger [" << loggerId << "]";
            rotate(loggerId);
            m_lastRotation[loggerId] = Utils::now();
            RLOG(INFO) << "[" << loggerId << "] rescheduled for rotation at [" << m_lastRotation[loggerId] << "]";
        } else {
            RLOG(DEBUG) << "Ignoring rotation for [" << loggerId << "] - Reason: " << " lastRotated="
                       << lastRotated << ", delta: " << (now - lastRotated) << " < " << (freq - LENIENCY_THRESHOLD);
        }
    }

    archiveRotatedItems();
}

void LogRotator::archiveRotatedItems()
{
    RVLOG_IF(!m_archiveItems.empty(), RV_DETAILS) << "Archiving rotated logs... [Total loggers: " << m_archiveItems.size() << "]";
    for (auto item : m_archiveItems) {
        std::thread t([&]() {
            el::Helpers::setThreadName("LogArchiver");
            RLOG(INFO) << "Archiving for [" << item.loggerId << "] => [" << item.archiveFilename
                       << "] containing " << item.files.size() << " files";
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

    for (auto f : files) {
        RVLOG(RV_DETAILS) << "Removing plain file [" << f.first << "] (" << Utils::bytesToHumanReadable(Utils::fileSize(f.first.c_str())) << ")";
        if (::remove(f.first.c_str()) != 0 ) {
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

unsigned long HourlyLogRotator::calculateRoundOff(unsigned long now) const
{
    return 3600 - (now % 3600);
}

unsigned long SixHoursLogRotator::calculateRoundOff(unsigned long now) const
{
    unsigned long minRoundOff = 3600 - (now % 3600);

    //
    // for six hours
    //now = 1517570594; // 22:23:14 - should should be 1 because roundoff=11 - 12 = 1
    //now = 1517579594; // 00:53:14 - should should be 5 because roundoff=1 - 6 = 5
    //now = 1517608394; // 08:53:14 - should should be 3 because roundoff=9 - 12 = 3
    //now = 1517626394; // 13:53:14 - should should be 4 because roundoff=2 - 6 = 4


    // setup based on minRoundOff and now
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

    //
    // for twelve hours
    //now = 1517570594; // 22:23:14 - should should be 1 because roundoff=23 - 24 = 1
    //now = 1517579594; // 00:53:14 - should should be 5+6=11 because roundoff=1 - 12 = 11
    //now = 1517608394; // 08:53:14 - should should be 3 because roundoff=9 - 12 = 3
    //now = 1517626394; // 13:53:14 - should should be 4+6=10 because roundoff=14 - 24 = 10


    // setup based on minRoundOff and now
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
    unsigned long minRoundOff = 3600 - (now % 3600);

    //
    // for daily (24 hours)
    //now = 1517570594; // 22:23:14 - should should be 1 because roundoff=23 - 24 = 1
    //now = 1517579594; // 00:53:14 - should should be 5+6+12=23 because roundoff=1 - 24 = 11
    //now = 1517608394; // 08:53:14 - should should be 3+12=15 because roundoff=9 - 24 = 15
    //now = 1517626394; // 13:53:14 - should should be 4+6=10 because roundoff=14 - 24 = 10


    // setup based on minRoundOff and now
    std::string next24HourStr = Utils::formatTime(minRoundOff + now, "%H");
    int next24Hour = atoi(next24HourStr.c_str());
    const std::map<std::string, int> WEEK_DAYS_MAP = {
        { "Mon", 1 },
        { "Tue", 2 },
        { "Wed", 3 },
        { "Thu", 4 },
        { "Fri", 5 },
        { "Sat", 6 },
        { "Sun", 7 }
    };
    std::string weekDayStr = Utils::formatTime(minRoundOff + now, "%a");
    int weekDay = WEEK_DAYS_MAP.at(weekDayStr);

    int hoursToNextIter = next24Hour == 0 || next24Hour == 12 ? 0 : 24 - next24Hour; // nearest day
    int secsToNextMidnight = minRoundOff + (hoursToNextIter * 3600);
    int daysToNextMonday = 7 - weekDay;
    return secsToNextMidnight + (daysToNextMonday * 86400);
}

unsigned long WeeklyLogRotator::calculateRoundOff(unsigned long now) const
{
    unsigned long minRoundOff = 3600 - (now % 3600);

    //
    // weekly
    //now = 1517574097; // Fri, 02/Feb/2018 23:21:27 - should should be 2 because min_roundoff = 24 (i.e, sat 00:00) sat->sun->mon = 2
    //now = 1517833297; // Mon, 05/Feb/2018 23:21:27 - should should be 6 because min_roundoff = 24 (i.e, tue 00:00) tue->wed->thu->fri->sat->sun->mon = 6
    //now = 1517660497; // Sun, 03/Feb/2018 23:21:27 - should should be 0 because next day is monday


    // setup based on minRoundOff and now
    std::string next24HourStr = Utils::formatTime(minRoundOff + now, "%H");
    int next24Hour = atoi(next24HourStr.c_str());
    const std::map<std::string, int> WEEK_DAYS_MAP = {
        { "Mon", 1 },
        { "Tue", 2 },
        { "Wed", 3 },
        { "Thu", 4 },
        { "Fri", 5 },
        { "Sat", 6 },
        { "Sun", 7 }
    };
    std::string weekDayStr = Utils::formatTime(minRoundOff + now, "%a");
    int weekDay = WEEK_DAYS_MAP.at(weekDayStr);

    // calculations
    int hoursToNextIter = next24Hour == 0 || next24Hour == 12 ? 0 : 24 - next24Hour; // nearest day
    int secsToNextMidnight = minRoundOff + (hoursToNextIter * 3600);
    int daysToNextMonday = 7 - weekDay;
    return secsToNextMidnight + (daysToNextMonday * 86400);
}

unsigned long MonthlyLogRotator::calculateRoundOff(unsigned long now) const
{
    unsigned long minRoundOff = 3600 - (now % 3600);
    //
    // monthly
    //now = 1517574097; // Fri, 02/Feb/2018 23:21:27 - should should be 26 because min_roundoff = 24 (i.e, sat 03/Feb) 28-3 = 25
    //now = 1519734097; // Tue, 27/Feb/2018 23:21:27 - should should be 1
    //now = 1519820497; // Wed, 28/Feb/2018 23:21:27 - should should be 0 because next day is next month


    // setup based on minRoundOff and now
    std::string next24HourStr = Utils::formatTime(minRoundOff + now, "%H");
    std::string monthDayStr = Utils::formatTime(minRoundOff + now, "%d");
    std::string yearStr = Utils::formatTime(minRoundOff + now, "%Y");
    std::string nextHourStr = Utils::formatTime(minRoundOff + now, "%h");
    std::string monthStr = Utils::formatTime(minRoundOff + now, "%M");
    int next24Hour = atoi(next24HourStr.c_str());
    int nextHour = atoi(nextHourStr.c_str());
    int monthDay = atoi(monthDayStr.c_str());
    int year = atoi(yearStr.c_str());
    int month = atoi(monthStr.c_str());
    const std::map<std::string, int> WEEK_DAYS_MAP = {
        { "Mon", 1 },
        { "Tue", 2 },
        { "Wed", 3 },
        { "Thu", 4 },
        { "Fri", 5 },
        { "Sat", 6 },
        { "Sun", 7 }
    };
    std::string weekDayStr = Utils::formatTime(minRoundOff + now, "%a");
    int weekDay = WEEK_DAYS_MAP.at(weekDayStr);
    const std::map<int, int> MONTH_MAX_DAYS_MAP = {
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
    int lastDayOfThisMonth = MONTH_MAX_DAYS_MAP.at(month);
    int hoursToNextIter = next24Hour == 0 || next24Hour == 12 ? 0 : 24 - next24Hour; // nearest day
    int secsToNextMidnight = minRoundOff + (hoursToNextIter * 3600);
    int daysToNextMonth = monthDay == lastDayOfThisMonth ? 0 : (lastDayOfThisMonth - monthDay);
    return secsToNextMidnight + (daysToNextMonth * 86400);
}

unsigned long YearlyLogRotator::calculateRoundOff(unsigned long now) const
{
    unsigned long minRoundOff = 3600 - (now % 3600);
    //
    // yearly
    //now = 1517574097; // Fri, 02/Feb/2018 23:21:27 - should be 9 (round off mar), extra days = 26


    // setup based on minRoundOff and now
    std::string next24HourStr = Utils::formatTime(minRoundOff + now, "%H");
    std::string monthDayStr = Utils::formatTime(minRoundOff + now, "%d");
    std::string yearStr = Utils::formatTime(minRoundOff + now, "%Y");
    std::string monthStr = Utils::formatTime(minRoundOff + now, "%M");
    int next24Hour = atoi(next24HourStr.c_str());
    int monthDay = atoi(monthDayStr.c_str());
    int year = atoi(yearStr.c_str());
    int month = atoi(monthStr.c_str());
    const std::map<int, int> MONTH_MAX_DAYS_MAP = {
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
    int lastDayOfThisMonth = MONTH_MAX_DAYS_MAP.at(month);

    int hoursToNextIter = next24Hour == 0 || next24Hour == 12 ? 0 : 24 - next24Hour; // nearest day
    int secsToNextMidnight = minRoundOff + (hoursToNextIter * 3600);
    int daysToNextMonth = lastDayOfThisMonth - monthDay;
    int secsToNextMonth = secsToNextMidnight + (daysToNextMonth * 86400);
    int monthsToNextYear = 12 - month;
    int extraDays = 0;
    for (int i = month + 1; i <= 12; ++i) {
        extraDays += MONTH_MAX_DAYS_MAP.at(i) - 28;
    }
    return secsToNextMonth + (monthsToNextYear * 28 * 86400) + (extraDays * 86400);
}
