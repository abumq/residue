//
//  session.cc
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

#include <iomanip>
#include <sstream>
#include "src/logging/log.h"
#include "src/net/session.h"
#include "src/core/registry.h"
#include "src/core/request-handler.h"
#include "src/core/configuration.h"
#include "src/core/response.h"
#include "src/crypto/base64.h"
#include "src/crypto/aes.h"
#include "src/crypto/rsa.h"

using namespace residue;

const std::string Session::PACKET_DELIMITER = "\r\n\r\n";
const std::size_t Session::PACKET_DELIMITER_SIZE = Session::PACKET_DELIMITER.size();

Session::Session(tcp::socket&& socket,
                 RequestHandler* requestHandler) :
    m_socket(std::move(socket)),
    m_requestHandler(requestHandler),
    m_bytesSent("0"),
    m_bytesReceived("0")
{
    DRVLOG(RV_DEBUG) << "New session " << this;
}

Session::~Session()
{
    DRVLOG(RV_DEBUG) << "End session " << this;
}

void Session::start()
{
    m_requestHandler->registry()->join(shared_from_this());
    read();
}

void Session::read()
{
    auto self(shared_from_this());
    boost::asio::async_read_until(m_socket, m_streamBuffer, Session::PACKET_DELIMITER,
                                  [&, this, self](boost::system::error_code ec, std::size_t numOfBytes) {

#ifdef RESIDUE_PROFILING
        //unsigned long m_timeTaken;
#endif
        if (!ec) {
            RESIDUE_PROFILE_START(t_read);
            std::istream is(&m_streamBuffer);
            std::string buffer((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
            buffer.erase(numOfBytes - Session::PACKET_DELIMITER_SIZE);
            //RESIDUE_PROFILE_CHECKPOINT(t_read, m_timeTaken, 1);
            sendToHandler(std::move(buffer));
            //RESIDUE_PROFILE_CHECKPOINT(t_read, m_timeTaken, 2);
            Utils::bigAdd(m_bytesReceived, std::to_string(numOfBytes));
            m_requestHandler->registry()->addBytesReceived(numOfBytes);
        } else {
            DRVLOG_IF(ec != boost::asio::error::eof, RV_DEBUG) << "Error: " << ec.message();
            m_requestHandler->registry()->leave(shared_from_this());
        }
    });
}

void Session::sendToHandler(std::string&& data)
{
    DRVLOG(RV_TRACE) << "Bytes: " << data;
    m_requestHandler->setSession(this);
    RawRequest req { std::move(data), m_socket.remote_endpoint().address().to_string(), Utils::now() };
    m_requestHandler->handle(std::move(req));
}

void Session::close()
{
    if (m_socket.is_open()) {
        DRVLOG(RV_DEBUG) << "Closing session...";
        m_socket.close();
    }
}

void Session::write(const char* data,
                    const char* key)
{
    std::string result = AES::encrypt(data, key);
    write(result.c_str(), result.size());
}

void Session::write(const char* data,
                    std::size_t,
                    const char* publicEncryptionKey)
{
    std::string encryptedData;
    try {
        encryptedData = RSA::encrypt(data, RSA::loadPublicKey(publicEncryptionKey));
#ifdef RESIDUE_USE_MINE
        std::string base64Encoded = Base64::encode(Base16::decode(encryptedData)) + Session::PACKET_DELIMITER;
#else // use ripe
        std::string base64Encoded = Base64::encode(encryptedData) + Session::PACKET_DELIMITER;
#endif
        write(base64Encoded.c_str(), base64Encoded.size());
    } catch (const std::exception& e) {
        RLOG(ERROR) << "PUBLIC KEY:" << std::endl << publicEncryptionKey;
        RLOG(ERROR) << "DATA:" << std::endl << data;
        std::string err = std::string(e.what()) + Session::PACKET_DELIMITER;
        write(err.c_str(), err.size());
    }
}

void Session::writeStatusCode(const Response::StatusCode& r)
{
    std::stringstream ss;
    if (r == Response::StatusCode::STATUS_OK || r == Response::StatusCode::CONTINUE) {
        ss << "{r:" << 0 << "}";
    } else {
        ss << "{r:" << 1 << ",c:" << static_cast<unsigned short>(r) << "}";
    }
    ss << Session::PACKET_DELIMITER;
    write(ss.str().c_str(), ss.str().length());
}

void Session::write(const std::string& s)
{
    write((s + Session::PACKET_DELIMITER).c_str(), s.size() + Session::PACKET_DELIMITER_SIZE);
}

void Session::write(const char* data,
                    std::size_t length)
{
    auto self(shared_from_this());
    Utils::bigAdd(m_bytesSent, std::to_string(length));
    m_requestHandler->registry()->addBytesSent(length);
    auto write = [&]() {
        DRVLOG(RV_DEBUG) << "Sending " << data;
        boost::asio::async_write(m_socket, boost::asio::buffer(data, length),
                                 [&, this, self](boost::system::error_code ec, std::size_t) {
            if (ec) {
                DRVLOG(RV_DEBUG) << "Failed to send." << ec.message();
                m_socket.close();
            }
        });
        read();
    };
    write();
}
