﻿//
//  residue-log-dispatcher.h
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

#ifndef ResidueLogDispatcher_h
#define ResidueLogDispatcher_h

#include <algorithm>
#include <mutex>
#include <unordered_map>

#include "core/configuration.h"
#include "extensions/log-extension.h"
#include "extensions/dispatch-error-extension.h"
#include "logging/log.h"
#include "logging/log-request.h"
#include "logging/user-message.h"
#include "non-copyable.h"
#include "utils/utils.h"

namespace residue {

class Configuration;

///
/// \brief Log dispatch callback that is core for residue
///
class ResidueLogDispatcher final : public el::LogDispatchCallback, NonCopyable
{
public:
    ///
    /// \brief FailedLog is used to locally store the log message
    /// in case of errors so we do not lose any data
    ///
    struct FailedLogs
    {
        el::Logger* logger;
        std::vector<std::string> lines;
    };

    ResidueLogDispatcher() :
        m_configuration(nullptr)
    {
    }

    inline void setConfiguration(Configuration* configuration)
    {
        m_configuration = configuration;
    }

    void handle(const el::LogDispatchData* data) override
    {
        el::LogDispatchCallback::handle(data);
        el::base::threading::ScopedLock scopedLock(fileHandle(data));

        try {
            if (data == nullptr) {
                // can't log here as we do not have logger information
                // to add 'residue' logger check
                std::cout << "Log dispatch data is unexpectedly null" << std::endl;
                return;
            }
            std::string logLine(data->logMessage()->logger()->logBuilder()->build(data->logMessage(), true));
            bool successfullyWritten = false;
            el::Logger* logger = data->logMessage()->logger();
            el::base::TypedConfigurations* conf = logger->typedConfigurations();
            el::Level level = data->logMessage()->level();
            if (conf->toFile(level)) {
                const std::string& fn = conf->filename(level);
                el::base::type::fstream_t* fs = conf->fileStream(level);
                if (fs != nullptr) {
                    if (!Utils::fileExists(fn.c_str())) {
                        RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, ERROR)
                                << "File not found [" << fn << "] [Logger: " << logger->id() << "]. Creating...";
                        if (!createFile(fn, fs, logger, logLine, level)) {
                            return;
                        }
                    }
                    fs->write(logLine.c_str(), logLine.size());
                    if (fs->fail()) {
                        RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, ERROR)
                                << "Failed to write to file [" << fn << "] [Logger: "
                                << logger->id() << "] " << std::strerror(errno);

                        addToDynamicBuffer(logger, fn, logLine);

                        execDispatchErrorExtensions(logger->id(),
                                                    fn,
                                                    logLine,
                                                    el::LevelHelper::castToInt(level),
                                                    errno);
                        if (logger->id() != RESIDUE_LOGGER_ID) {
                            // recovery check for dynamic buffer
                            std::ofstream oftmp(fn.c_str(), std::ios::out | std::ios::app);
                            if (oftmp.is_open()) {
                                oftmp << "=== [residue] ==> dynamic buffer recovery check ===\n";
                                oftmp.flush();
                                if (!oftmp.fail()) {
                                    fs->clear();
                                    RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, INFO) << "Dynamic buffer recovery check passed for [" << fn << "]";
                                }
                                oftmp.close();
                            }
                        }
                    } else {
                        if (ELPP->hasFlag(el::LoggingFlag::ImmediateFlush) || (logger->isFlushNeeded(level))) {
                            logger->flush(level, fs);
                        }
                        successfullyWritten = true;

                        dispatchDynamicBuffer(fn, fs, logger);

                        if (m_previouslyFailed && logger->id() != RESIDUE_LOGGER_ID) {
                            resetErrorExtensions(); // this resets m_previouslyFailed as well
                        }
                    }
                } else {
                    RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, ERROR)
                            << "Log file (FILENAME) for ["
                            << el::LevelHelper::convertToString(level)
                             << "] level has not been configured but [TO_FILE] is configured to TRUE. [Logger: "
                             << logger->id() << "]";
                }
            }
#ifdef RESIDUE_HAS_EXTENSIONS
            if (data->logMessage()->logger()->id() != RESIDUE_LOGGER_ID) {
                execLogExtensions(data, logLine, successfullyWritten);
            }
#endif
        } catch (const std::exception& e) {
            std::cerr << "Unexpected exception: " << e.what() << std::endl;
        }
    }

private:
    Configuration* m_configuration;
    // map of filename -> FailedLogs
    std::unordered_map<std::string, FailedLogs> m_dynamicBuffer;
    std::recursive_mutex m_dynamicBufferLock;
    std::atomic<bool> m_previouslyFailed;

    friend class Stats;

    void execLogExtensions(const el::LogDispatchData* data,
                           const el::base::type::string_t& logLine,
                           bool successfullyWritten)
    {
        if (m_configuration->logExtensions().empty()) {
            return;
        }
        const UserMessage* logMessage = static_cast<const UserMessage*>(data->logMessage());
        LogExtension::Data d {
            el::LevelHelper::castToInt(logMessage->request()->level()),
            logMessage->request()->applicationName(),
            logMessage->request()->threadId(),
            logMessage->request()->filename(),
            logMessage->request()->lineNumber(),
            logMessage->request()->function(),
            logMessage->request()->verboseLevel(),
            logMessage->logger()->id(),
            logMessage->request()->clientId(),
            logMessage->request()->ipAddr(),
            logMessage->request()->sessionId(),
            logMessage->request()->msg(),
            logLine,
            successfullyWritten
        };
        for (auto& ext : m_configuration->logExtensions()) {
            ext->trigger(&d);
        }
    }

    void execDispatchErrorExtensions(const std::string& loggerId,
                                     const std::string& filename,
                                     const el::base::type::string_t& logLine,
                                     unsigned int level,
                                     int errorNo)
    {
        if (m_configuration->dispatchErrorExtensions().empty()
                || loggerId == RESIDUE_LOGGER_ID) {
            return;
        }
        DispatchErrorExtension::Data d {
            loggerId,
            filename,
            logLine,
            level,
            errorNo
        };
        for (auto& ext : m_configuration->dispatchErrorExtensions()) {
            ext->trigger(&d);
        }
        m_previouslyFailed = true;
    }

    void resetErrorExtensions()
    {
        if (m_configuration->dispatchErrorExtensions().empty()) {
            return;
        }
        for (auto& ext : m_configuration->dispatchErrorExtensions()) {
            static_cast<DispatchErrorExtension*>(ext)->reset();
        }
        m_previouslyFailed = false;
    }

    void addToDynamicBuffer(el::Logger* logger, const std::string& filename, const std::string& logLine)
    {
        if (m_configuration->hasFlag(Configuration::ENABLE_DYNAMIC_BUFFER)
                && logger->id() != RESIDUE_LOGGER_ID) {
            // never add logs to dynamic buffer for residue logger as dynamic
            // buffer may be locked
            std::lock_guard<std::recursive_mutex> lock(m_dynamicBufferLock);
            if (m_dynamicBuffer.find(filename) == m_dynamicBuffer.end()) {
                m_dynamicBuffer.insert(std::make_pair(filename, FailedLogs {
                                                          logger,
                                                          std::vector<std::string>()
                                                      }));
            }
            if (std::find(m_dynamicBuffer[filename].lines.begin(), m_dynamicBuffer[filename].lines.end(), logLine)
                    == m_dynamicBuffer[filename].lines.end()) {
                m_dynamicBuffer[filename].lines.push_back(logLine);
            }
        }
    }

    void dispatchDynamicBuffer(const std::string& fn, el::base::type::fstream_t* fs, el::Logger* logger)
    {
        if (m_dynamicBuffer.find(fn) != m_dynamicBuffer.end()) {
            RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, ERROR)
                    << "This logger [" << logger->id() << "] has some data in dynamic buffer [" << fn << "]"
                    << " Flushing all the messages first [" << m_dynamicBuffer[fn].lines.size() << " items]";
            std::lock_guard<std::recursive_mutex> lock(m_dynamicBufferLock);
            std::vector<std::string>* list = &m_dynamicBuffer[fn].lines;
            if (!list->empty()) {
                // lock the logger
                el::base::threading::ScopedLock loggerLock(m_dynamicBuffer[fn].logger->lock());
                // close/open again
                fs->close();
                fs->open(fn, std::ios::out | std::ios::app);
                auto size = list->size();
                auto dynamicBufferClearStart = std::chrono::high_resolution_clock::now();
                *fs << "=== [residue] ==> " << size << " log" << (size > 1 ? "s" : "") << " from dynamic buffer ===\n";
                for (auto& line : *list) {
                    // process all items
                    fs->write(line.c_str(), line.size());
                    if (fs->fail()) {
                        RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, ERROR) << "Dynamic buffer dispatch failed [" << fn << "] "
                                                                          << std::strerror(errno);

                    }
                }
                auto dynamicBufferClearEnd = std::chrono::high_resolution_clock::now();
                *fs << "=== [residue] ==> dynamic buffer cleared (" << size << " log"
                    << (size > 1 ? "s" : "") << " written in "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(dynamicBufferClearEnd - dynamicBufferClearStart).count()
                    << " ms) ===\n";
                fs->flush();
            }
            m_dynamicBuffer.erase(fn);
        }
    }

    bool createFile(const std::string& fn,
                    el::base::type::fstream_t* fs,
                    el::Logger* logger,
                    const std::string& logLine,
                    el::Level level)
    {
        if (Utils::createPath(el::base::utils::File::extractPathFromFilename(fn).c_str())) {
            RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, INFO) << "Accessing file...";
            fs->close();
            fs->open(fn, std::ios::out);
            Utils::updateFilePermissions(fn.data(), logger, m_configuration);
            if (fs->fail() || !fs->is_open()) {
                RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, INFO)
                        << "Failed to access file [ " << fn << "]! " << std::strerror(errno);

                addToDynamicBuffer(logger, fn, logLine);

                fs->clear();

                execDispatchErrorExtensions(logger->id(),
                                            fn,
                                            logLine,
                                            el::LevelHelper::castToInt(level),
                                            errno);
                return false;
            }
        } else {
            RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, ERROR)
                    << "Failed to create file [" << fn << "] [Logger: "
                    << logger->id() << "] " << std::strerror(errno);

            addToDynamicBuffer(logger, fn, logLine);

            execDispatchErrorExtensions(logger->id(),
                                        fn,
                                        logLine,
                                        el::LevelHelper::castToInt(level),
                                        errno);
            return false;
        }
        return true;
    }
  };
}

#endif /* ResidueLogDispatcher_h */
