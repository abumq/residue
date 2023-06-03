//
//  server.h
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
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

#ifndef Server_h
#define Server_h

#include "net/asio.h"
#include "non-copyable.h"

using net::ip::tcp;

namespace residue {

class RequestHandler;

///
/// \brief Server containing abstract request handler that determines request
/// handler at constructor time and calls the handler with new session
///
class Server final : NonCopyable
{
public:
    Server(int port, RequestHandler* requestHandler);
    ~Server();

    void start();

private:
    void accept();

    net::io_service m_ioService;
    tcp::acceptor m_acceptor;
    tcp::socket m_socket;

    RequestHandler* m_requestHandler;
};
}
#endif /* Server_h */
