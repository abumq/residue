//
//  residue-log-dispatcher.h
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

#ifndef ResidueLogDispatcher_h
#define ResidueLogDispatcher_h

#include "extensions/log-extension.h"
#include "extensions/dispatch-error-extension.h"
#include "logging/log.h"
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
    ResidueLogDispatcher() : m_configuration(nullptr)
    {
    }

    inline void setConfiguration(Configuration* configuration)
    {
        m_configuration = configuration;
    }

public:
    void handle(const el::LogDispatchData* data) override
    {
        el::LogDispatchCallback::handle(data);
        el::base::threading::ScopedLock scopedLock(fileHandle(data));

        m_data = data;
        try {
            if (m_data == nullptr) {
                // don't log anything here
                // if you do, add a check for residue logger
                std::cout << "Log dispatch data is unexpectedly null" << std::endl;
                return;
            }
            std::string logLine(data->logMessage()->logger()->logBuilder()->build(data->logMessage(), true));

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
                        if (Utils::createPath(el::base::utils::File::extractPathFromFilename(fn).c_str())) {
                            RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, INFO) << "Accessing file...";
                            fs->close();
                            fs->open(fn, std::ios::out);
                            Utils::updateFilePermissions(fn.data(), logger, m_configuration);
                            if (fs->fail() || !fs->is_open()) {
                                RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, INFO)
                                        << "Failed to access file [ " << fn << "]! " << std::strerror(errno);

                                execDispatchErrorExtensions(logger->id(),
                                                            fn,
                                                            el::LevelHelper::castToInt(level),
                                                            errno);
                                return;
                            }
                        } else {
                            RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, ERROR)
                                    << "Failed to create file [" << fn << "] [Logger: "
                                    << logger->id() << "] " << std::strerror(errno);
                            execDispatchErrorExtensions(logger->id(),
                                                        fn,
                                                        el::LevelHelper::castToInt(level),
                                                        errno);
                            return;
                        }
                    }
                    fs->write(logLine.c_str(), logLine.size());
                    if (fs->fail()) {
                        RLOG_IF(logger->id() != RESIDUE_LOGGER_ID, ERROR)
                                << "Failed to write to file [" << fn << "] [Logger: "
                                << logger->id() << "] " << std::strerror(errno);

                        execDispatchErrorExtensions(logger->id(),
                                                    fn,
                                                    el::LevelHelper::castToInt(level),
                                                    errno);
                    } else {
                        if (ELPP->hasFlag(el::LoggingFlag::ImmediateFlush) || (logger->isFlushNeeded(level))) {
                            logger->flush(level, fs);
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
                execLogExtensions(data, logLine);
            }
#endif
        } catch (const std::exception& e) {
            std::cerr << "Unexpected exception: " << e.what() << std::endl;
        }
    }

private:
    const el::LogDispatchData* m_data;
    Configuration* m_configuration;

    void execLogExtensions(const el::LogDispatchData* data,
                        const el::base::type::string_t& logLine)
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
            logLine
        };
        for (auto& ext : m_configuration->logExtensions()) {
            ext->trigger(&d);
        }
    }

    void execDispatchErrorExtensions(const std::string& loggerId,
                                     const std::string& filename,
                                     unsigned int level,
                                     int errorNo)
    {
        if (m_configuration->dispatchErrorExtensions().empty()) {
            return;
        }
        DispatchErrorExtension::Data d {
            loggerId,
            filename,
            level,
            errorNo
        };
        for (auto& ext : m_configuration->dispatchErrorExtensions()) {
            ext->trigger(&d);
        }
    }
  };
}

#endif /* ResidueLogDispatcher_h */
