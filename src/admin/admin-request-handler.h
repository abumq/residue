//
//  admin-request-handler.h
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

#ifndef AdminRequestHandler_h
#define AdminRequestHandler_h

#include "core/request-handler.h"

namespace residue {

class CommandHandler;
///
/// \brief Handles incoming AdminRequest
/// \see AdminRequest
///
class AdminRequestHandler : public RequestHandler
{
public:
    explicit AdminRequestHandler(Registry*, CommandHandler*);
    virtual void handle(RawRequest&&);
private:
    CommandHandler* m_commandHandler;

    void respond(const std::string&, const std::shared_ptr<Session>& session) const;
};
}

#endif /* AdminRequestHandler_h */
