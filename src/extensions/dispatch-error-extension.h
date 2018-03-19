//
//  dispatch-error-extension.h
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

#ifndef DispatchErrorExtension_h
#define DispatchErrorExtension_h

#include <string>
#include "extensions/extension.h"

namespace residue {

///
/// \brief Base class for any extension that needs to execute a code
/// if there is a dispatch error
///
class DispatchErrorExtension : public Extension
{
public:
    ///
    /// \brief With each execution pointer to this data is passed in
    /// to take advantage of it
    ///
    struct Data
    {
        ///
        /// \brief Logger ID for the dispatch
        ///
        std::string loggerId;

        ///
        /// \brief File where log was being dispatched to
        ///
        std::string filename;

        ///
        /// \brief Log line that was being dispatched
        ///
        std::string logLine;

        ///
        /// \brief Logging level
        ///
        unsigned int level;

        ///
        /// \brief Last error number (errno)
        /// \see http://en.cppreference.com/w/cpp/error/errno_macros
        /// \see http://en.cppreference.com/w/cpp/string/byte/strerror
        ///
        int errorNumber;
    };


    explicit DispatchErrorExtension(const std::string& id);
    virtual ~DispatchErrorExtension() = default;

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
