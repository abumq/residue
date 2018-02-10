//
//  token-request-handler.h
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

#ifndef TokenRequestHandler_h
#define TokenRequestHandler_h

#include "core/request-handler.h"

namespace residue {

///
/// \brief Handles incoming token request
/// \see TokenRequest
///
class TokenRequestHandler final : public RequestHandler
{
public:
    explicit TokenRequestHandler(Registry*);
    virtual void handle(RawRequest&&);
};
}

#endif /* TokenRequestHandler_h */
