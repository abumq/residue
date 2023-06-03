//
//  server.cc
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

#include "net/server.h"

#include <thread>

#include "net/asio.h"

#include "core/registry.h"
#include "core/request-handler.h"
#include "logging/log.h"
#include "net/session.h"

using namespace residue;
using net::ip::tcp;

Server::Server(int port, RequestHandler* requestHandler) :
    m_acceptor(m_ioService, tcp::endpoint(tcp::v4(), port)),
    m_socket(m_ioService),
    m_requestHandler(requestHandler)
{
    accept();
}

Server::~Server()
{
    if (m_socket.is_open()) {
        m_socket.close();
    }
}

void Server::accept()
{
    m_acceptor.async_accept(m_socket, [this](residue::error_code ec) {
        if (!ec) {
            std::make_shared<Session>(std::move(m_socket), m_requestHandler)->start();
        }
        accept();
    });
}

void Server::start()
{
    m_ioService.run();
}
