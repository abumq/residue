//
//  user-log-builder.cc
//  Residue
//
//  Copyright 2017-present Amrayn Web Services
//  https://amrayn.com
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

#include "logging/user-log-builder.h"

#include "logging/log-request.h"
#include "logging/user-message.h"

using namespace residue;

// same as Easylogging++ base::consts::k....
static const char* kAppNameFormatSpecifier               =      "%app";
static const char* kLoggerIdFormatSpecifier              =      "%logger";
static const char* kThreadIdFormatSpecifier              =      "%thread";
static const char* kSeverityLevelFormatSpecifier         =      "%level";
static const char* kSeverityLevelShortFormatSpecifier    =      "%levshort";
static const char* kDateTimeFormatSpecifier              =      "%datetime";
static const char* kLogFileFormatSpecifier                 =      "%file";
static const char* kLogFileBaseFormatSpecifier             =      "%fbase";
static const char* kLogLineFormatSpecifier                =      "%line";
static const char* kLogLocationFormatSpecifier            =      "%loc";
static const char* kLogFunctionFormatSpecifier      =      "%func";
static const char* kCurrentUserFormatSpecifier      =      "%user";
static const char* kCurrentHostFormatSpecifier      =      "%host";
static const char* kMessageFormatSpecifier          =      "%msg";
static const char* kVerboseLevelFormatSpecifier     =      "%vlevel";

el::base::type::string_t UserLogBuilder::build(const el::LogMessage* msg,
                                               bool appendNewLine) const
{

#ifdef RESIDUE_HIGH_RESOLUTION_PROFILING
   types::Time m_timeTakenLogBuilder;
   RESIDUE_PROFILE_START(t_log_builder);
#endif
    const UserMessage* logMessage = static_cast<const UserMessage*>(msg);
    el::base::TypedConfigurations* tc = logMessage->logger()->typedConfigurations();
    if (tc == nullptr || logMessage->request() == nullptr) {
        // DO NOT LOG ANYTHING HERE!
        std::cout << "Unexpectedly NULL request!, msg => [" << logMessage->message() << "]" << std::endl;
        return "";
    }

    const el::base::LogFormat* logFormat = &(tc->logFormat(logMessage->request()->level()));
    el::base::type::string_t logLine = logFormat->format();
    char buff[el::base::consts::kSourceFilenameMaxLength + el::base::consts::kSourceLineMaxLength] = "";
    const char* bufLim = buff + sizeof(buff);

    RESIDUE_HIGH_PROFILE_CHECKPOINT_NS(t_log_builder, m_timeTakenLogBuilder, 1, 1);
    if (logFormat->hasFlag(el::base::FormatFlags::AppName)) {
        // App name
        el::base::utils::Str::replaceFirstWithEscape(logLine, kAppNameFormatSpecifier,
                                                 logMessage->request()->applicationName());
    }
    if (logFormat->hasFlag(el::base::FormatFlags::ThreadId)) {
        // Thread ID
        el::base::utils::Str::replaceFirstWithEscape(logLine, kThreadIdFormatSpecifier,
                                                     logMessage->request()->threadId());
    }
    if (logFormat->hasFlag(el::base::FormatFlags::DateTime)) {
        // DateTime
        el::base::utils::Str::replaceFirstWithEscape(logLine, kDateTimeFormatSpecifier, logMessage->request()->formattedDatetime(logFormat->dateTimeFormat().c_str(), &tc->millisecondsWidth(logMessage->request()->level())));
    }
    if (logFormat->hasFlag(el::base::FormatFlags::Function)) {
        // Function
        el::base::utils::Str::replaceFirstWithEscape(logLine, kLogFunctionFormatSpecifier, logMessage->request()->function());
    }
    if (logFormat->hasFlag(el::base::FormatFlags::File)) {
        // File
        el::base::utils::Str::clearBuff(buff, el::base::consts::kSourceFilenameMaxLength);
        el::base::utils::File::buildStrippedFilename(logMessage->request()->filename().c_str(), buff);
        el::base::utils::Str::replaceFirstWithEscape(logLine, kLogFileFormatSpecifier, std::string(buff));
    }
    if (logFormat->hasFlag(el::base::FormatFlags::FileBase)) {
        // FileBase
        el::base::utils::Str::clearBuff(buff, el::base::consts::kSourceFilenameMaxLength);
        el::base::utils::File::buildBaseFilename(logMessage->request()->filename(), buff);
        el::base::utils::Str::replaceFirstWithEscape(logLine, kLogFileBaseFormatSpecifier, std::string(buff));
    }
    if (logFormat->hasFlag(el::base::FormatFlags::Line)) {
        // Line
        char* buf = el::base::utils::Str::clearBuff(buff, el::base::consts::kSourceLineMaxLength);
        buf = el::base::utils::Str::convertAndAddToBuff(logMessage->request()->lineNumber(), el::base::consts::kSourceLineMaxLength, buf, bufLim, false);
        el::base::utils::Str::replaceFirstWithEscape(logLine, kLogLineFormatSpecifier, std::string(buff));
    }
    if (logFormat->hasFlag(el::base::FormatFlags::Location)) {
        // Location
        char* buf = el::base::utils::Str::clearBuff(buff,
                                                el::base::consts::kSourceFilenameMaxLength + el::base::consts::kSourceLineMaxLength);
        el::base::utils::File::buildStrippedFilename(logMessage->request()->filename().c_str(), buff);
        buf = el::base::utils::Str::addToBuff(buff, buf, bufLim);
        buf = el::base::utils::Str::addToBuff(":", buf, bufLim);
        buf = el::base::utils::Str::convertAndAddToBuff(logMessage->request()->lineNumber(),  el::base::consts::kSourceLineMaxLength, buf, bufLim,
                                                    false);
        el::base::utils::Str::replaceFirstWithEscape(logLine, kLogLocationFormatSpecifier, std::string(buff));
    }
    if (logMessage->request()->level() == el::Level::Verbose && logFormat->hasFlag(el::base::FormatFlags::VerboseLevel)) {
        // Verbose level
        char* buf = el::base::utils::Str::clearBuff(buff, 1);
        buf = el::base::utils::Str::convertAndAddToBuff(logMessage->request()->verboseLevel(), 1, buf, bufLim, false);
        el::base::utils::Str::replaceFirstWithEscape(logLine, kVerboseLevelFormatSpecifier, std::string(buff));
    }
    if (logFormat->hasFlag(el::base::FormatFlags::LogMessage)) {
        // Log message
        el::base::utils::Str::replaceFirstWithEscape(logLine, kMessageFormatSpecifier, logMessage->request()->msg());
    }

    RESIDUE_HIGH_PROFILE_CHECKPOINT_NS(t_log_builder, m_timeTakenLogBuilder, 2, 1);

    el::base::utils::Str::replaceFirstWithEscape(logLine, "%client_id", logMessage->request()->clientId());
    el::base::utils::Str::replaceFirstWithEscape(logLine, "%ip", logMessage->request()->ipAddr());
    el::base::utils::Str::replaceFirstWithEscape(logLine, "%session_id", logMessage->request()->sessionId());

#if !defined(ELPP_DISABLE_CUSTOM_FORMAT_SPECIFIERS)
    for (auto cfs : *ELPP->customFormatSpecifiers()) {
        std::string fs(cfs.formatSpecifier());
        el::base::type::string_t wcsFormatSpecifier(fs.begin(), fs.end());
        el::base::utils::Str::replaceFirstWithEscape(logLine, wcsFormatSpecifier, std::string(cfs.resolver()(logMessage)));
    }
#endif  // !defined(ELPP_DISABLE_CUSTOM_FORMAT_SPECIFIERS)

    if (appendNewLine) {
        logLine += ELPP_LITERAL("\n");
    }
    RESIDUE_HIGH_PROFILE_CHECKPOINT_NS(t_log_builder, m_timeTakenLogBuilder, 3, 2);
    return logLine;
}
