//
//  post-archive-extension.h
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

#ifndef PostArchiveExtension_h
#define PostArchiveExtension_h

#include <string>
#include "extensions/extension.h"

namespace residue {

///
/// \brief Base class for any extension that needs to execute a code
/// after log rotation was successful and we have .tar.gz file available
///
class PostArchiveExtension : public Extension
{
public:
    ///
    /// \brief With each execution pointer to this data is passed in
    /// to take advantage of it
    ///
    struct Data
    {
        ///
        /// \brief Logger ID for which archive was created
        ///
        std::string loggerId;

        ///
        /// \brief Full path to created archive
        ///
        std::string archiveFilename;
    };

    explicit PostArchiveExtension(const std::string& id);
    virtual ~PostArchiveExtension() = default;

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

#endif /* PostArchiveExtension_h */
