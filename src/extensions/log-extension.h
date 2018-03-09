//
//  log-extension.h
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

#ifndef LogExtension_h
#define LogExtension_h

#include <string>
#include "extensions/extension.h"

namespace residue {

///
/// \brief Base class for any extension that needs to execute a code
/// with every dispatch of user logs
///
/// Data is well defined and contains all the necessary information about
/// the log message
///
class LogExtension : public Extension
{
public:
    ///
    /// \brief With each execution pointer to this data is passed in
    /// to take advantage of it
    ///
    struct Data
    {
        ///
        /// \brief Logging level
        ///
        unsigned int level;

        ///
        /// \brief Application ID if specified
        ///
        std::string app;

        ///
        /// \brief Thread ID where log message was generated from
        ///
        std::string thread;

        ///
        /// \brief Source file of the log message
        ///
        std::string file;

        ///
        /// \brief Source line of the log message
        ///
        unsigned long int line;

        ///
        /// \brief Source function of the log message
        ///
        std::string func;

        ///
        /// \brief Verbose level if applicable
        ///
        unsigned short verboseLevel;

        ///
        /// \brief Logger ID for the log message
        ///
        std::string loggerId;

        ///
        /// \brief Client ID for the log message
        ///
        std::string clientId;

        ///
        /// \brief IP address where log request was sent from
        ///
        std::string ipAddr;

        ///
        /// \brief Session by which server received this log request
        ///
        std::string sessionId;

        ///
        /// \brief Log message without any format
        ///
        std::string message;

        ///
        /// \brief Full formatted message (this is the message that was appended to the log file)
        ///
        std::string formattedMessage;

        ///
        /// \brief True if log was dispatched to the file without any error
        ///
        bool successfullyWritten;
    };

    ///
    /// \brief Represents typed level. Statically cast Data::level to this
    /// for readibility purposes
    ///
    enum class Level : unsigned int
    {
        Trace = 2,
        Debug = 4,
        Fatal = 8,
        Error = 16,
        Warning = 32,
        Verbose = 64,
        Info = 128
    };

    explicit LogExtension(const std::string& id);
    virtual ~LogExtension() = default;

    ///
    /// \brief The pure virtual function that must be implemented by the extension
    ///
    /// \param data The data is passed in as constant pointer. Residue extension API does not
    /// control the 'delete' over this pointer. No one should ever delete this data as other
    /// extensions may need it
    ///
    virtual Extension::Result execute(const Data* const data) = 0;

private:
    virtual Extension::Result executeWrapper(void* d) override;
};
}

#endif /* LogExtension_h */
