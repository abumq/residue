//
//  connection-request-handler.h
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

#ifndef ConnectionRequestHandler_h
#define ConnectionRequestHandler_h

#include "core/request-handler.h"

namespace residue {
class ConnectionRequest;

///
/// \brief Handle incoming ConnectRequest
/// \see ConnectionRequest
///
class ConnectionRequestHandler : public RequestHandler
{
public:
    explicit ConnectionRequestHandler(Registry*);
    virtual void handle(RawRequest&&);
private:
    void connect(ConnectionRequest*, const std::shared_ptr<Session>&, bool isManagedClient) const;
    void acknowledge(const ConnectionRequest*, const std::shared_ptr<Session>&) const;
    void touch(const ConnectionRequest*, const std::shared_ptr<Session>&) const;
};
}

#endif /* ConnectionRequestHandler_h */
