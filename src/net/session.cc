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
#include "logging/log.h"
#include "net/session.h"
#include "core/registry.h"
#include "core/request-handler.h"
#include "core/configuration.h"
#include "core/response.h"
#include "crypto/base64.h"
#include "crypto/aes.h"
#include "crypto/rsa.h"

using namespace residue;

const std::string Session::PACKET_DELIMITER = "\r\n\r\n";
const std::size_t Session::PACKET_DELIMITER_SIZE = Session::PACKET_DELIMITER.size();

Session::Session(tcp::socket&& socket,
                 RequestHandler* requestHandler) :
    m_socket(std::move(socket)),
    m_requestHandler(requestHandler),
    m_client(nullptr),
    m_bytesSent("0"),
    m_bytesReceived("0")
{
    m_id = m_requestHandler->name()[0] + Utils::generateRandomString(16, true);
    DRVLOG(RV_DEBUG) << "New session " << m_id;
}

Session::~Session()
{
    DRVLOG(RV_DEBUG) << "End session " << m_id;
}

void Session::start()
{
    m_requestHandler->registry()->join(shared_from_this());
    read();
}

void Session::read()
{
    auto self(shared_from_this());
    net::async_read_until(m_socket, m_streamBuffer, Session::PACKET_DELIMITER,
                                  [&, this, self](residue::error_code ec, std::size_t numOfBytes) {

#ifdef RESIDUE_PROFILING
        //types::Time m_timeTaken;
#endif
        if (!ec) {
            RESIDUE_PROFILE_START(t_read);
#ifdef RESIDUE_DEV
            DRVLOG(RV_TRACE) << "Received: "
                             << (numOfBytes - Session::PACKET_DELIMITER_SIZE) // ignore package delimiter
                             << " bytes";
#endif
            std::istream is(&m_streamBuffer);
            std::string buffer((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
            buffer.erase(numOfBytes - Session::PACKET_DELIMITER_SIZE);
            //RESIDUE_PROFILE_CHECKPOINT(t_read, m_timeTaken, 1);
            sendToHandler(std::move(buffer));
            //RESIDUE_PROFILE_CHECKPOINT(t_read, m_timeTaken, 2);
            if (m_requestHandler->registry()->configuration()->hasFlag(Configuration::ENABLE_CLI)) {
#ifdef RESIDUE_DEV
                DRVLOG(RV_TRACE) << "Adding bytes";
#endif
                Utils::bigAdd(m_bytesReceived, std::to_string(numOfBytes));
                m_requestHandler->registry()->addBytesReceived(numOfBytes);
            }
        } else {
#ifdef RESIDUE_DEBUG
            DRVLOG_IF(ec != net::error::eof, RV_DEBUG) << "Error: " << ec.message();
#endif
            m_requestHandler->registry()->leave(shared_from_this());
        }
    });
}

void Session::sendToHandler(std::string&& data)
{
#ifdef RESIDUE_DEBUG
    DRVLOG(RV_TRACE) << "Read bytes: " << data << " [size: " << data.size() << "]";
#endif
    RawRequest req {
        std::move(data),
        m_socket.remote_endpoint().address().to_string(),
        Utils::now(),
        shared_from_this()
    };
    m_requestHandler->handle(std::move(req));
}

void Session::close()
{
    if (m_socket.is_open()) {
#ifdef RESIDUE_DEBUG
        DRVLOG(RV_DEBUG) << "Closing session...";
#endif
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

void Session::writeStandardResponse(const Response::StatusCode& r)
{
    write(Response::STANDARD_RESPONSES[r].response.c_str(),
          Response::STANDARD_RESPONSES[r].response.length());
}

void Session::write(const std::string& s)
{
    write((s + Session::PACKET_DELIMITER).c_str(),
          s.size() + Session::PACKET_DELIMITER_SIZE);
}

void Session::write(const char* data,
                    std::size_t length)
{
 //   auto self(shared_from_this());
    if (m_requestHandler->registry()->configuration()->hasFlag(Configuration::ENABLE_CLI)) {
        Utils::bigAdd(m_bytesSent, std::to_string(length));
        m_requestHandler->registry()->addBytesSent(length);
    }

#ifdef RESIDUE_DEBUG
    DRVLOG(RV_DEBUG_2) << "Sending " << data;
#endif
    net::async_write(m_socket, net::buffer(data, length),
                     [&, this/*, self*/](residue::error_code ec, std::size_t) {
        if (ec) {
#ifdef RESIDUE_DEBUG
            DRVLOG(RV_DEBUG) << "Failed to send. " << ec.message();
#endif

            // auto destroy socket if needed
            // do not close here
            // https://github.com/muflihun/residue/issues/79
        }
    });
    read();
}
