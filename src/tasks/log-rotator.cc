//
//  log-rotator.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include <ctime>
#include <cstdio>
#include <iterator>
#include <unordered_set>
#include <vector>
#include <set>
#include <thread>
#include "include/log.h"
#include "src/utils/utils.h"
#include "src/tasks/log-rotator.h"
#include "src/core/registry.h"
#include "src/core/configuration.h"
#include "src/crypto/zlib.h"

using namespace residue;

const unsigned long LogRotator::LENIENCY_THRESHOLD = 60 * 5; // 5 minutes

LogRotator::LogRotator(Registry* registry,
                       unsigned int interval) :
    Task("LogRotator", registry, interval, true)
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
        el::base::SubsecondPrecision ssPrec(3);
        struct timeval tval;
        std::stringstream ss;
        ss << "Scheduled to run ";
        if (lastRotated == 0L) {
            ss << "in next execution (";
            tval.tv_sec = static_cast<long>(nextExecution());
            ss << el::base::utils::DateTime::timevalToString(tval, "%d %b, %Y %H:%m:%s", &ssPrec);
            ss << ")";
        } else {
            unsigned long nextRotation = lastRotated + freq;
            bool skippedLast = false;
            if (Utils::now() > nextRotation) {
                // if next rotation is already past (just in case)
                // show the freq after this one
                nextRotation = nextRotation + freq;
                skippedLast = true;
            }
            tval.tv_sec = static_cast<long>(nextRotation);
            ss << "at " << el::base::utils::DateTime::timevalToString(tval, "%d %b, %Y %H:%m:%s", &ssPrec);
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
            std::string loggerId = std::get<0>(item);
            std::string archiveFilename = std::get<1>(item);
            std::map<std::string, std::string> files = std::get<2>(item);
            el::Helpers::setThreadName("LogArchiver");
            RLOG(INFO) << "Archiving for [" << loggerId << "] => [" << archiveFilename
                       << "] containing " << files.size() << " files";
            archiveAndCompress(loggerId, archiveFilename, files);
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
    m_archiveItems.push_back(std::make_tuple(loggerId, destinationDir + el::base::consts::kFilePathSeperator + archiveFilename, files));
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