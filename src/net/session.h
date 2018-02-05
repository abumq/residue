//
//  session.h
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

#ifndef Session_h
#define Session_h

#include <boost/asio.hpp>
#include "src/core/response.h"

using boost::asio::ip::tcp;

namespace residue {
class RequestHandler;
class Registry;

///
/// \brief Session object with each connection
///
class Session final : public std::enable_shared_from_this<Session>
{
public:

    static const std::string PACKET_DELIMITER;
    static const std::size_t PACKET_DELIMITER_SIZE;

    Session(tcp::socket&& socket, RequestHandler* requestHandler);
    ~Session();

    ///
    /// \brief start new session
    ///
    void start();

    ///
    /// \brief Write encrypted data using symmetric encryption
    ///
    void write(const char* data, const char* key);

    ///
    /// \brief Write encrypted data using asymmetric encryption
    ///
    void write(const char* data, std::size_t, const char* publicEncryptionKey);

    ///
    /// \brief Write status code to client without any contents
    ///
    void writeStatusCode(const Response::StatusCode& r);

    ///
    /// \brief Writes plain (formats with <length>:<content>)
    ///
    void write(const std::string& s);

    ///
    /// \brief Returns socket by const reference
    ///
    inline const tcp::socket& socket() const
    {
        return m_socket;
    }

    inline const std::string& bytesReceived() const
    {
        return m_bytesReceived;
    }

    inline const std::string& bytesSent() const
    {
        return m_bytesSent;
    }
private:

    ///
    /// \brief Read incoming data and calls sendToHandler on the packet
    ///
    void read();

    ///
    /// \brief close session
    ///
    void close();

    ///
    /// \brief Write plain data to the client
    ///
    void write(const char* data, std::size_t length);

    ///
    /// \brief Send the packet bytes to the handler
    ///
    void sendToHandler(std::string&& incomingBytes);

    tcp::socket m_socket;
    RequestHandler* m_requestHandler;
    std::string m_name;
    boost::asio::streambuf m_streamBuffer;

    std::string m_bytesSent;
    std::string m_bytesReceived;
};
}

#endif /* Session_h */
