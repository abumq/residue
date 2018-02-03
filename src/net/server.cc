//
//  server.cc
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

#include <thread>
#include <boost/asio.hpp>
#include "src/logging/log.h"
#include "src/net/server.h"
#include "src/net/session.h"
#include "src/core/request-handler.h"
#include "src/core/registry.h"

using namespace residue;
using boost::asio::ip::tcp;

Server::Server(boost::asio::io_service& io_service, int port, RequestHandler* requestHandler) :
    m_acceptor(io_service, tcp::endpoint(tcp::v4(), port)),
    m_socket(io_service),
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
    m_acceptor.async_accept(m_socket, [this](boost::system::error_code ec) {
        if (!ec) {
            std::make_shared<Session>(std::move(m_socket), m_requestHandler)->start();
        }
        accept();
    });
}
