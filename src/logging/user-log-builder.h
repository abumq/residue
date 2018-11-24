//
//  user-log-builder.h
//  Residue
//
//  Copyright 2017-present Zuhd Web Services
//  https://zuhd.org
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

#ifndef UserLogBuilder_h
#define UserLogBuilder_h

#include "logging/log.h"
#include "non-copyable.h"

namespace residue {

///
/// \brief Custom log builder for Residue
///
class UserLogBuilder final : public el::LogBuilder, NonCopyable
{

public:
    virtual el::base::type::string_t build(const el::LogMessage* logMessage,
                                   bool appendNewLine) const override;
};
}

#endif /* UserLogBuilder_h */
