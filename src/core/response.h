//
//  response.h
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

#ifndef Response_h
#define Response_h

#include <string>
#include "non-copyable.h"
#include "core/json-object.h"

namespace residue {

///
/// \brief Simple response
///
class Response : NonCopyable
{
public:

    enum StatusCode : unsigned short
    {
        STATUS_OK = 0,
        BAD_REQUEST = 1,
        CONTINUE = 0
    };

    struct StandardResponse
    {
        StatusCode code;
        std::string response;
    };

    static const StandardResponse STANDARD_RESPONSES[];

    Response() = default;
    virtual ~Response() = default;

    void serialize(JsonObject::Json& root,
                   std::string& output) const;
};
}
#endif /* Response_h */
