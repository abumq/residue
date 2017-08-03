//
//  Residue.cc
//
//  Official C++ client library for Residue logging server
//
//  Copyright © 2017 Muflihun Labs. All rights reserved.
//
//  https://muflihun.com
//  https://muflihun.github.io/residue
//  https://github.com/muflihun
//

#include <ctime>
#include <cstdlib>
#include <memory>
#include <chrono>
#include <functional>
#include <tuple>
#include <boost/asio.hpp>
#include <Ripe.h>
#include "include/Residue.h"
#include "include/json.h"
#include "include/log.h"

INITIALIZE_EASYLOGGINGPP

const std::size_t Residue::DEFAULT_KEY_SIZE = 2048;
const std::string Residue::LOCALHOST = "127.0.0.1";
const int Residue::DEFAULT_PORT = 8777;
const unsigned int Residue::TOUCH_THRESHOLD = 120; // should always be min(client_age)
const std::string Residue::DEFAULT_ACCESS_CODE = "default";

using json = nlohmann::json;
using boost::asio::ip::tcp;

///
/// \brief Internal logging class. We do not use Easylogging++ macros here
/// for they will cause issue when dispatching
///
class reslog
{
public:
    enum Level {
        crazy,   // 0
        debug,   // 1
        info,    // 2
        warning, // 3
        error,   // 4
        none     // 5
    };

    reslog(Level level)
        : m_level(level),
          m_enabled(level >= Residue::s_internalLoggingLevel)
    {
    }

    template <typename T>
    reslog& operator<<(const T& item)
    {
        if (m_enabled) {
            m_stream << item;
        }
        return *this;
    }

    virtual ~reslog()
    {
        if (m_enabled) {
            std::string levelStr;
            switch (m_level) {
            case crazy:
                levelStr = "CRAZY";
                break;
            case debug:
                levelStr = "DEBUG";
                break;
            case error:
                levelStr = "ERROR";
                break;
            case warning:
                levelStr = "WARNING";
                break;
            case info:
                levelStr = "INFO";
                break;
            default:
            case none:
                break;
            }

            std::cout << "[" << levelStr << "]: " << m_stream.str() << std::endl;
        }
    }
private:
    std::stringstream m_stream;
    Level m_level;
    bool m_enabled;
};

#define RESIDUE_LOCK_LOG(id) reslog(reslog::crazy) << id << "_lock()"; reslog scoped_reslog(reslog::crazy); scoped_reslog << id << "_unlock()";

volatile int Residue::s_internalLoggingLevel = reslog::none;

///
/// \brief Residue client connector class.
/// This uses Boost ASIO for internet and Ripe for encryption
///
class ResidueClient
{
public:
    ///
    /// Response handle from server. std::string = responseBody, bool = hasError, , std::string&& = errorText
    ///
    using ResponseHandler = std::function<void(std::string&&, bool, std::string&&)>;

    ///
    /// \brief ResidueClient constructor
    /// \param host Residue server host
    /// \param port Residue server connect_port (rvalue)
    ///
    ResidueClient(const std::string& host, std::string&& port) noexcept;

    virtual ~ResidueClient() noexcept = default;

    ///
    /// \brief Sends payload to the server
    /// \param payload Payload to be sent
    /// \param expectResponse If true, it waits for response from the server. Use this
    /// only when we know the application is going to be alive by the time async_wait
    /// can finish. The only time we can be potentially sure about this situation is when
    /// running this in a gui application
    /// \param retryOnTimeout Retry by connect()ing and the sending again
    /// \param resHandler When response received this callback is called
    /// \see ResponseHandler
    ///
    void send(std::string&& payload, bool expectResponse = false,
              const ResponseHandler& resHandler = [](std::string&&, bool, std::string&&) -> void {}, bool retryOnTimeout = true, bool ignoreQueue = false);

    ///
    /// \brief Whether connected to the server (using this host and port) or not
    /// \return True if connected, otherwise false
    ///
    inline bool connected() const noexcept { return m_socket.is_open() && m_connected; }

    ///
    /// \brief Returns last error recorded
    ///
    inline const std::string& lastError() const noexcept { return m_lastError; }

    ///
    /// \brief Connects to the server using initial host and port
    ///
    void connect();
private:
    std::string m_host;
    std::string m_port;
    boost::asio::io_service m_ioService;
    tcp::socket m_socket;
    bool m_connected;
    std::string m_lastError;

    ///
    /// \brief Reads incomming bytes from the server
    /// \param resHandler When response is read, this callback is called
    /// \see ResponseHandler
    ///
    void read(const ResponseHandler& resHandler = [](std::string&&, bool, std::string&&) -> void {}, const std::string& payload = "");
};

static std::unique_ptr<ResidueClient> s_connectionClient;
static std::unique_ptr<ResidueClient> s_tokenClient;
static std::unique_ptr<ResidueClient> s_loggingClient;


ResidueClient::ResidueClient(const std::string& host, std::string&& port) noexcept :
    m_host(host),
    m_port(std::move(port)),
    m_socket(m_ioService),
    m_connected(false)
{
}

void ResidueClient::connect()
{
    tcp::resolver resolver(m_ioService);
    tcp::resolver::iterator endpoint = resolver.resolve(tcp::resolver::query(m_host, m_port));
    // Do not use async_connect as we want client application to wait
    try {
        boost::asio::connect(m_socket, endpoint);
        m_connected = true;
    } catch (const std::exception& e) {
        m_lastError = "Failed to connect: " + std::string(e.what());
        reslog(reslog::error) << m_lastError;
        throw ResidueException(e.what());
    }
}

void ResidueClient::send(std::string&& payload, bool expectResponse, const ResponseHandler& resHandler, bool retryOnTimeout, bool ignoreQueue)
{
    reslog(reslog::crazy) << "Send: " << payload;
    if (!m_socket.is_open()) {
        reslog(reslog::error) << "Socket closed [" << m_port << "]";
        return;
    }
    if (!m_connected) {
        reslog(reslog::error) << "Client not connected [" << m_port << "]";
        return;
    }
    reslog(reslog::crazy) << "Writing [" << m_port << "]..." << (expectResponse ? "" : " (async)");
    if (expectResponse) {
        try {
            boost::asio::write(m_socket, boost::asio::buffer(payload, payload.length()));
            reslog(reslog::crazy) << "Waiting for response...";
            read(resHandler, payload);
            reslog(reslog::crazy) << "Done";
        } catch (const std::exception& e) {
            m_lastError = "Failed to send: " + std::string(e.what());
            reslog(reslog::error) << m_lastError;
            if (retryOnTimeout && m_lastError.find("Operation timed out") != std::string::npos) {
                m_socket.close();
                m_connected = false;
                connect();
                send(std::move(payload), expectResponse, resHandler, false);
            } else {
                throw ResidueException(e.what());
            }
        }
    } else {
        boost::asio::async_write(m_socket, boost::asio::buffer(payload, payload.length()),
                                 [&](const boost::system::error_code&, std::size_t) {
        });
    }
}

void ResidueClient::read(const ResponseHandler& resHandler, const std::string& payload)
{
    boost::system::error_code ec;
    boost::asio::streambuf buf;
    try {
        std::size_t numOfBytes = boost::asio::read_until(m_socket, buf, Ripe::PACKET_DELIMITER, ec);
        if (!ec) {
            std::istream is(&buf);
            std::string buffer((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
            buffer.erase(numOfBytes - Ripe::PACKET_DELIMITER_SIZE);
            resHandler(std::move(buffer), false, "");
        } else {
            if ((ec == boost::asio::error::eof) || (ec == boost::asio::error::connection_reset)) {
                reslog(reslog::error) << "Failed to send, requeueing...";
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                m_connected = false;
            } else {
                reslog(reslog::error) << ec.message();
                resHandler("", true, ec.message());
            }
        }
    } catch (const std::exception& e) {
        reslog(reslog::error) << e.what();
        resHandler("", true, std::string(e.what()));
    }
}

class ResidueDispatcher : public el::base::DefaultLogDispatchCallback // we override default log dispatcher rather than create new one
{
public:
    ResidueDispatcher() : m_residue(nullptr), m_data(nullptr) {}

    inline void setResidue(Residue* residue) { m_residue = residue; }
protected:
    void handle(const el::LogDispatchData* data) noexcept override;
private:
    Residue* m_residue;
    const el::LogDispatchData* m_data;
    std::string m_host;
    int m_port;
};

void ResidueDispatcher::handle(const el::LogDispatchData* data) noexcept
{
    m_data = data;

    if (data->logMessage()->logger()->id() == RESIDUE_LOGGER_ID) {
        el::base::DefaultLogDispatchCallback::handle(m_data);
    } else {
        std::time_t t = std::time(nullptr);
        std::tm* nowTm;
        if (m_residue->m_utc) {
            nowTm = std::gmtime(&t);
        } else {
            nowTm = std::localtime(&t);
        }
        time_t now;
        if (nowTm != nullptr) {
            now = mktime(nowTm);
            now += m_residue->m_timeOffset;
            now *= 1000;
        } else {
            reslog(reslog::error) << "Unable to create time. Using second method to send local time instead.";
            now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
        }
        m_residue->addToQueue(std::move(std::make_tuple(now,
                                                        el::Helpers::getThreadName(),
                                                        m_data->logMessage()->logger()->id(),
                                                        m_data->logMessage()->message(),
                                                        m_data->logMessage()->file(),
                                                        m_data->logMessage()->line(),
                                                        m_data->logMessage()->func(),
                                                        static_cast<unsigned int>(m_data->logMessage()->level()),
                                                        m_data->logMessage()->verboseLevel()
                                                        )
                                        )
                              );
    }
}

void residueCrashHandler(int sig) noexcept
{
    reslog(reslog::error) << "Crashed!";
    if (!Residue::instance().crashHandlerLoggerId().empty()) {
        CLOG(ERROR, Residue::instance().crashHandlerLoggerId().c_str()) << "Crashed!";
        el::Helpers::logCrashReason(sig, true, el::Level::Fatal, Residue::instance().crashHandlerLoggerId().c_str());
    }
    Residue::disconnect();
    el::Helpers::crashAbort(sig);
}

Residue::Residue() noexcept :
    m_host(""),
    m_port(0),
    m_accessCodeMap(nullptr),
    m_connected(false),
    m_connecting(false),
    m_disconnected(false),
    m_bulkDispatch(false),
    m_bulkSize(1),
    m_maxBulkSize(1),
    m_plainRequest(false),
    m_age(0),
    m_dateCreated(0),
    m_loggingPort(0),
    m_tokenPort(0),
    m_serverFlags(0x0),
    m_running(false),
    m_knownClient(false),
    m_keySize(DEFAULT_KEY_SIZE),
#ifdef RESIDUE_PROFILING
    queue_ms(0), dispatch_ms(0), c_total(0),
#endif
    m_utc(false),
    m_timeOffset(0),
    m_dispatchDelay(1),
    m_autoBulkParams(true),
    m_dispatcher(nullptr)
{
    reslog(reslog::debug) << "Initialized residue!";
}

Residue::~Residue() noexcept
{
    Residue::disconnect();
}

void Residue::healthCheck() noexcept
{
    if (!isClientValid()) {
        try {
            reset();
        } catch (const std::exception& e) {
            // We ignore these errors as we have
            // log messages from the reset()
        }
    } else if (shouldTouch()) {
        touch();
    }
}

void Residue::connect_(const std::string& host, int port, AccessCodeMap* accessCodeMap)
{
    if (m_connected) {
        reslog(reslog::error) << "Already connected. Please disconnect first";
        return;
    }
    reslog(reslog::info) << "Connecting...";

    m_disconnected = false; // This method is only called by user so we want to reset disconnected state
    m_host = host;
    m_port = port;
    m_accessCodeMap = accessCodeMap;

    el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);

    s_connectionClient = std::unique_ptr<ResidueClient>(new ResidueClient(m_host, std::to_string(m_port)));

    try {
        s_connectionClient->connect();
        if (!s_connectionClient->connected()) {
            addError("Failed to connect. (Port: " + std::to_string(m_port) + ") " + s_connectionClient->lastError());
            throw ResidueException(m_errors.at(m_errors.size() - 1));
        }
    } catch (const ResidueException& e) {
        throw e;
    }

    if (!m_knownClient) {
        // Generate new key pair
#ifdef RESIDUE_PROFILING
        TIMED_SCOPE(timed, "gen_rsa");
#endif // ELPP_DEBUG_LOG
        Ripe::KeyPair rsaKey = Ripe::generateRSAKeyPair(m_keySize);
        m_rsaPrivateKey = rsaKey.privateKey;
        m_rsaPublicKey = rsaKey.publicKey;
    }
    reslog(reslog::info) << "Estabilishing full connection...";
    reset();

}

void Residue::onConnect() noexcept
{
    reslog(reslog::info) << "Successfully connected";

    // Register callbacks (and unregister default callback)
    el::Helpers::uninstallLogDispatchCallback<el::base::DefaultLogDispatchCallback>("DefaultLogDispatchCallback");
    el::Helpers::installLogDispatchCallback<ResidueDispatcher>("ResidueDispatcher");
    ResidueDispatcher* dispatcher = el::Helpers::logDispatchCallback<ResidueDispatcher>("ResidueDispatcher");
    dispatcher->setEnabled(true);
    dispatcher->setResidue(this);

    if (!m_running) {
        // we do not want to cancel existing thread so we use m_running as reference point
        m_running = true;
        delete m_dispatcher;
        m_dispatcher = nullptr; // just in case if next line fails due to insufficient memory
        m_dispatcher = new std::thread([&]() {
            while (m_running) {
                dispatch();
                if ((!m_connected || m_disconnected) && m_requests.empty()) {
                    disconnect_();
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(m_dispatchDelay));
            }
            reslog(reslog::debug) << "Finishing residue...";
        });
    }
}

void Residue::reset()
{
    reslog(reslog::debug) << "Resetting connection...";
    m_connecting = true;
    m_connected = false;
    RESIDUE_LOCK_LOG("reset");
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    m_errors.clear();
    m_tokens.clear();
    int type = 1; // 1 = CONNECT

    json j;
    j["type"] = type;
    j["_t"] = getTimestamp();
    if (!m_knownClient) {
        j["rsa_public_key"] = Ripe::base64Encode(m_rsaPublicKey);
    } else {
        j["client_id"] = m_clientId;
    }

    reslog(reslog::debug) << "Connecting...(1)";

    std::string data = j.dump();
    if (!m_serverPublicKey.empty()) {
        data = Ripe::base64Encode(Ripe::encryptRSA(data, m_serverPublicKey));
    }
    data += Ripe::PACKET_DELIMITER;
    try {
        s_connectionClient->send(std::move(data), true,
                                 [&](std::string&& connectResponse, bool hasError, std::string&& errorText) -> void {
            m_connecting = false;
            if (hasError || connectResponse.empty()) {
                reslog(reslog::error) << "Failed to connect!";
                addError("Failed to connect to the residue server. " + errorText);
                throw ResidueException(m_errors.at(m_errors.size() - 1));
            } else {
                reslog(reslog::debug) << "Connecting...(2)";
                std::string decryptedResponse;
                if (connectResponse.at(0) == '{') {
                    decryptedResponse = connectResponse;
                } else {
                    try {
                        decryptedResponse = Ripe::decryptRSA(connectResponse, m_rsaPrivateKey, true, false, m_rsaPrivateKeySecret);
                    } catch (const std::exception& e) {
                        reslog(reslog::error) << "Failed to read response: " << e.what();
                        addError("Client private key does not match with the corresponding public key on the server.");
                        throw ResidueException(m_errors.at(m_errors.size() - 1));
                    }
                }

                int result = decryptedResponse.empty() ? -1 : 0;
                if (result != -1) {

                    reslog(reslog::debug) << "Connecting...(2) > Parsing...";

                    try {
                        j = json::parse(decryptedResponse);
                        if (j.count("status") == 1 && j["status"].get<int>() == 0) {
                            m_key = j["key"].get<std::string>();

                            // This two-step setting is necessary otherwise setting it directly to m_clientId gives
                            // memory error:
                            //
                            // malloc: *** error for object 0x10e5bd1c0: pointer being freed was not allocated
                            // *** set a breakpoint in malloc_error_break to debug
                            //
                            std::string clientId = j["client_id"].get<std::string>();
                            m_clientId = clientId;
                            m_connecting = true;
                            reslog(reslog::debug) << "Connecting...(3)";
                            json j2;
                            type = 2; // 2 = ACKNOWLEDGE
                            j2["type"] = type;
                            j2["_t"] = getTimestamp();
                            j2["client_id"] = m_clientId;
                            std::string jsonRequest = j2.dump();
                            std::string ackRequest = Ripe::prepareData(jsonRequest.c_str(), m_key, m_clientId.c_str());
                            try {
                                s_connectionClient->send(ackRequest.c_str(), true, [&](std::string&& ackResponse, bool hasError, std::string&& errorText) -> void {
                                    reslog(reslog::debug) << "Saving connection parameters...";
                                    m_connecting = false;
                                    if (hasError) {
                                        reslog(reslog::error) << "Failed to connect. Network error";
                                        addError("Failed to connect to the residue server. Network error. " + errorText);
                                        throw ResidueException(m_errors.at(m_errors.size() - 1));
                                    } else {
                                        if (ackResponse.find("{") != std::string::npos) {
                                            // Error
                                            if (ackResponse.at(0) != '{') {
                                                // remove head
                                                ackResponse.erase(0, ackResponse.find_first_of(":") + 1);
                                            }
                                            try {
                                                j = json::parse(ackResponse);
                                                reslog(reslog::error) << j["error_text"].get<std::string>();
                                                addError(j["error_text"].get<std::string>());
                                                throw ResidueException(m_errors.at(m_errors.size() - 1));
                                            } catch (const std::exception& e) {
                                                reslog(reslog::error) << "Error occurred but could not parse response: " << ackResponse << " (" << e.what() << ")";
                                            }

                                        } else {
                                            std::string iv;
                                            try {
                                                std::string decryptedAckResponse = Ripe::decryptAES(ackResponse, m_key, iv, true);
                                                j = json::parse(decryptedAckResponse);
                                                m_key = j["key"].get<std::string>();
                                                m_age = j["age"].get<unsigned int>();
                                                m_dateCreated = j["date_created"].get<unsigned long>();
                                                m_loggingPort = j["logging_port"].get<int>();
                                                m_tokenPort = j["token_port"].get<int>();
                                                m_maxBulkSize = j["max_bulk_size"].get<unsigned int>();
                                                m_serverFlags = j["flags"].get<unsigned int>();

                                                // Token server
                                                s_tokenClient = std::unique_ptr<ResidueClient>(new ResidueClient(m_host, std::to_string(m_tokenPort)));
                                                try {
                                                    s_tokenClient->connect();
                                                    if (!s_tokenClient->connected()) {
                                                        addError("Failed to connect. (Port: " + std::to_string(m_tokenPort) + ") " + s_tokenClient->lastError());
                                                        throw ResidueException(m_errors.at(m_errors.size() - 1));
                                                    }
                                                } catch (const ResidueException& e) {
                                                    addError("Failed to connect. (Port: " + std::to_string(m_tokenPort) + ") " + e.what());
                                                    throw e;
                                                }

                                                // Logging server
                                                s_loggingClient = std::unique_ptr<ResidueClient>(new ResidueClient(m_host, std::to_string(m_loggingPort)));
                                                try {
                                                    s_loggingClient->connect();
                                                    if (!s_loggingClient->connected()) {
                                                        addError("Failed to connect. (Port: " + std::to_string(m_loggingPort) + ") " + s_loggingClient->lastError());
                                                        throw ResidueException(m_errors.at(m_errors.size() - 1));
                                                    }
                                                } catch (const ResidueException& e) {
                                                    addError("Failed to connect. (Port: " + std::to_string(m_loggingPort) + ") " + e.what());
                                                    throw e;
                                                }

                                                if (m_autoBulkParams) {
                                                    if (hasFlag(Flag::ALLOW_BULK_LOG_REQUEST)) {
                                                        m_bulkDispatch = true;
                                                        m_bulkSize = std::min(m_maxBulkSize, 40U);
                                                    }
                                                }

                                                if (hasFlag(Flag::ALLOW_BULK_LOG_REQUEST) && m_bulkDispatch && m_bulkSize > m_maxBulkSize) {
                                                    addError(std::string("Invalid bulk dispatch size. Maximum allowed: " + std::to_string(m_maxBulkSize)));
                                                    throw ResidueException(m_errors.at(m_errors.size() - 1));
                                                } else if (!hasFlag(Flag::ALLOW_BULK_LOG_REQUEST) && m_bulkDispatch) {
                                                    addError("Bulk log requests not allowed by this server");
                                                    throw ResidueException(m_errors.at(m_errors.size() - 1));
                                                }
                                                m_connected = true;

                                                onConnect();
                                            } catch (const std::exception& e) {
                                                reslog(reslog::error) << "Failed to connect (4): " << e.what();
                                                throw ResidueException(e.what());
                                            }
                                        }
                                    }
                                });
                            } catch (const ResidueException e) {
                                throw e;
                            }
                        } else {
                           reslog(reslog::error) << "Failed to connect (2): " << j["error_text"].get<std::string>();
                           addError("Failed to connect. " + j["error_text"].get<std::string>());
                           throw ResidueException(m_errors.at(m_errors.size() - 1));
                        }
                    } catch (const std::exception& e) {
                        reslog(reslog::error) << "Failed to connect (2) [parse error]: " << e.what();
                        throw ResidueException(e.what());
                    }

                } else {
                    reslog(reslog::debug) << "Failed to read response!";
                    reslog(reslog::error) << "Failed to connect (2)";
                    if (m_knownClient) {
                        reslog(reslog::error) << "RSA private key provided does not match the public key on the server.";
                        addError("RSA private key provided does not match the public key on the server.");
                        throw ResidueException(m_errors.at(m_errors.size() - 1));
                    }
                }
            }
        });
    } catch (const ResidueException& e) {
        throw e;
    }
}

void Residue::addToQueue(RequestTuple&& request) noexcept
{
#ifdef RESIDUE_PROFILING
    c_total++;
    unsigned long queue_dura;
    RESIDUE_PROFILE_START(t_queue);
#endif
    RESIDUE_LOCK_LOG("addToQueue");
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_requests.push_front(std::move(request));

#ifdef RESIDUE_PROFILING
    RESIDUE_PROFILE_END(t_queue, queue_dura);
    ResidueLogger(ResidueLogger::debug) << queue_dura << "ms taken to add to queue";
    queue_ms += queue_dura;
#endif
}

void Residue::dispatch()
{
#ifdef RESIDUE_PROFILING
    int c = 0;
    int c_in_while = 0;
    unsigned long while_loop;
    unsigned long for_loop;
    unsigned long prepare_data;
    unsigned long send_recv;
    unsigned long total_duration_dispatch;
#endif

    RESIDUE_PROFILE_START(t_dispatch);
    bool plain = m_plainRequest && hasFlag(Flag::ALLOW_PLAIN_LOG_REQUEST);
    while (!m_requests.empty()) {

        while (m_connecting) {
            reslog(reslog::debug) << "Still connecting...";
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        try {
            if (!s_loggingClient->connected() || !s_tokenClient->connected()) {
                reslog(reslog::info) << "Trying to connect...";
                m_connected = false;
                connect();
            }
        } catch (const ResidueException& e) {
            reslog(reslog::error) << "Failed to connect: " << e.what() << ". Retrying in 500ms";
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }

        RESIDUE_PROFILE_START(t_while);
        std::string jsonData;

        if (m_bulkDispatch) {
            jsonData.append("[");
        }
        std::size_t totalRequests = static_cast<std::size_t>(m_bulkDispatch ? m_bulkSize : 1);
        totalRequests = std::min(totalRequests, m_requests.size());

        // Health check once per bulk
        healthCheck();

        RESIDUE_PROFILE_CHECKPOINT(t_dispatch, total_duration_dispatch, 1);

        RESIDUE_PROFILE_START(t_for);
        for (std::size_t i = 0; i < totalRequests; ++i) {

            RequestTuple request;

            {
                RESIDUE_LOCK_LOG("dispatch");
                std::lock_guard<std::recursive_mutex> lock(m_mutex);
                request = m_requests.back();
                m_requests.pop_back();
            }
            jsonData.append(requestToJson(std::move(request)));
            if (i < totalRequests - 1) {
                jsonData.append(",");
            }
            RESIDUE_PROFILE_CHECKPOINT(t_dispatch, total_duration_dispatch, 2);

#ifdef RESIDUE_PROFILING
            c_dispatched++;
            c++;
#endif
        }

        RESIDUE_PROFILE_END(t_for, for_loop);

        if (m_bulkDispatch) {
            jsonData.append("]");
        }
        RESIDUE_PROFILE_CHECKPOINT(t_dispatch, total_duration_dispatch, 3);

        RESIDUE_PROFILE_START(t_prepare_data);

        if (hasFlag(Flag::COMPRESSION)) {
            try {
                jsonData = Ripe::base64Encode(Ripe::compressString(jsonData));
            } catch (const std::exception& e) {
                reslog(reslog::error) << "Failed to compress the data: " << e.what();
            }
        }

        std::string data = plain ? std::move(jsonData) + Ripe::PACKET_DELIMITER : Ripe::prepareData(jsonData.c_str(), m_key, m_clientId.c_str());
        RESIDUE_PROFILE_END(t_prepare_data, prepare_data);

        RESIDUE_PROFILE_START(t_send);
        s_loggingClient->send(std::move(data), true); // we keep it to true as we may end program before it's actually dispatched

        RESIDUE_PROFILE_END(t_send, send_recv);

#ifdef RESIDUE_PROFILING
        RESIDUE_PROFILE_END(t_while, while_loop);
        c_in_while++;
        RESIDUE_PROFILE_CHECKPOINT(t_dispatch, total_duration_dispatch, 4);
        ResidueLogger(ResidueLogger::debug) << "While: " << while_loop << " ms For: " << for_loop << " ms Prepare: " << prepare_data << " ms send_recv " << send_recv << "ms \n";
#endif
    }

#ifdef RESIDUE_PROFILING
    RESIDUE_PROFILE_END(t_dispatch, total_duration_dispatch);
    ResidueLogger(ResidueLogger::debug) << total_duration_dispatch << " ms taken for dispatching " << c << " items with " << c_in_while << " each loop\n";

    dispatch_ms += total_duration_dispatch;
#endif
}

std::string Residue::requestToJson(RequestTuple&& request)
{
    unsigned long date = std::get<0>(request);
    std::string& threadId = std::get<1>(request);
    std::string& loggerId = std::get<2>(request);
    el::base::type::string_t& msg = std::get<3>(request);
    std::string& file = std::get<4>(request);
    el::base::type::LineNumber line = std::get<5>(request);
    std::string& func = std::get<6>(request);
    unsigned int level = std::get<7>(request);
    el::base::type::VerboseLevel vlevel = std::get<8>(request);

    json j;
    if (!m_applicationId.empty()) {
        j["app"] = m_applicationId;
    }

    if (hasFlag(Flag::CHECK_TOKENS)) {
        std::string token = getToken(loggerId);
        if (token.empty()) {
            obtainToken(loggerId);
        }
        token = getToken(loggerId);

        j["token"] = token;
    }

    if (m_plainRequest && hasFlag(Flag::ALLOW_PLAIN_LOG_REQUEST)) {
        j["client_id"] = m_clientId;
    }

    j["thread"] = threadId;
    j["datetime"] = date;
    j["logger"] = loggerId;
    j["msg"] = msg;
    if (!file.empty()) {
        j["file"] = file;
    }
    if (line > 0L) {
        j["line"] = line;
    }
    if (!func.empty()) {
        j["func"] = func;
    }
    j["level"] = level;

    if (vlevel > 0) {
        j["vlevel"] = vlevel;
    }
    return j.dump();
}

bool Residue::shouldTouch() const noexcept
{
    if (m_age == 0) {
        // Always alive so don't need to touch the client
        return false;
    }
    unsigned long now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
    return m_age - (now - m_dateCreated) < Residue::TOUCH_THRESHOLD;
}

void Residue::touch() noexcept
{
    reslog(reslog::debug) << "Touching...";
    m_connecting = true;
    json j;
    int type = 3; // 3 = TOUCH
    j["type"] = type;
    j["_t"] = getTimestamp();
    j["client_id"] = m_clientId;
    std::string jsonRequest = j.dump();
    std::string request;
    try {
        request = Ripe::prepareData(jsonRequest.c_str(), m_key, m_clientId.c_str());
    } catch (const std::exception& e) {
        reslog(reslog::error) << "Failed to prepare request (TOUCH): " << e.what();
    }

    s_connectionClient->send(std::move(request), true, [&](std::string&& response, bool hasError, std::string&& errorText) -> void {
        m_connecting = false;
        if (hasError) {
            reslog(reslog::error) << "Failed to touch. Network error. " + errorText;
        } else {
            if (response.find("{") != std::string::npos) {
                // Error
                if (response.at(0) != '{') {
                    // remove head
                    response.erase(0, response.find_first_of(":") + 1);
                }
                try {
                    j = json::parse(response);
                    reslog(reslog::error) << j["error_text"].get<std::string>();
                    addError(j["error_text"].get<std::string>());
                } catch (const std::exception& e) {
                    reslog(reslog::error) << "Error occurred but could not parse response: " << response << " (" << e.what() << ")";
                }

            } else {
                std::string iv;
                try {
                    std::string decryptedResponse = Ripe::decryptAES(response, m_key, iv, true);
                    j = json::parse(decryptedResponse);
                    RESIDUE_LOCK_LOG("sendTouch");
                    std::lock_guard<std::recursive_mutex> lock(m_mutex);
                    m_age = j["age"].get<int>();
                    m_dateCreated = j["date_created"].get<unsigned long>();
                } catch (const std::exception& e) {
                    reslog(reslog::error) << "Failed to read TOUCH response: " << e.what();
                }
            }
        }
    });
}

void Residue::disconnect_() noexcept
{
    reslog(reslog::debug) << "Disconnecting...";
    m_connected = false;
    el::Helpers::installLogDispatchCallback<el::base::DefaultLogDispatchCallback>("DefaultLogDispatchCallback");
    el::Helpers::uninstallLogDispatchCallback<ResidueDispatcher>("ResidueDispatcher");
}

void Residue::disconnect() noexcept
{
    if (Residue::connected()) {
        Residue::instance().m_connected = false;
        Residue::instance().m_disconnected = true;
        Residue::wait();
        Residue::instance().m_tokens.clear();
        Residue::instance().m_running = false;
    }
}

bool Residue::isClientValid() const noexcept
{
    if (m_age == 0) {
        return true;
    }
    unsigned long now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
    return now - m_dateCreated < m_age;
}

void Residue::obtainTokens()
{
    if (m_accessCodeMap != nullptr) {
        for (auto& accessCodeMap : *m_accessCodeMap) {
            std::string loggerId = accessCodeMap.first;
            std::string accessCode = accessCodeMap.second;
            obtainToken(loggerId, accessCode);
        }
    }
}

bool Residue::hasToken(const std::string& loggerId) const noexcept
{
    bool available = m_tokens.find(loggerId) != m_tokens.end();
    if (available) {
        // Check validity
        Token t = m_tokens.at(loggerId);
        if (t.life != 0) {
            unsigned long now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
            if (now - t.dateCreated > t.life) {
                available = false;
            }
        }
    }
    return available;
}

std::string Residue::getToken(const std::string& loggerId) const
{
    if (hasToken(loggerId)) {
        return m_tokens.at(loggerId).token;
    }
    return std::string();
}

void Residue::obtainToken(const std::string& loggerId)
{
    std::string accessCode = DEFAULT_ACCESS_CODE;
    if (m_accessCodeMap != nullptr
            && m_accessCodeMap->find(loggerId) != m_accessCodeMap->end()) {
        accessCode = m_accessCodeMap->at(loggerId);
    }
    obtainToken(loggerId, accessCode);
}

void Residue::obtainToken(const std::string& loggerId, const std::string& accessCode)
{
    if (accessCode == DEFAULT_ACCESS_CODE && !hasFlag(Flag::ALLOW_DEFAULT_ACCESS_CODE)) {
        throw ResidueException("Loggers without access code are not allowed by the server [Logger ID: " + loggerId + "]");
    }
    reslog(reslog::debug) << "Obtaining token for [" << loggerId << "] using [" << accessCode << "]";
    json j;
    j["_t"] = getTimestamp();
    j["logger_id"] = loggerId;
    j["access_code"] = accessCode;
    std::string jsonRequest = j.dump();
    std::string request = Ripe::prepareData(jsonRequest.c_str(), m_key, m_clientId.c_str());
    try {
        s_tokenClient->send(std::move(request), true, [&](std::string&& response, bool hasError, std::string&& errorText) -> void {
            if (hasError) {
                reslog(reslog::error) << "Failed to obtain token. " << errorText;
            } else {
                if (response.find("{") != std::string::npos) {
                    // Error
                    if (response.at(0) != '{') {
                        // remove head
                        response.erase(0, response.find_first_of(":") + 1);
                    }
                    try {
                        j = json::parse(response);
                        reslog(reslog::error) << j["error_text"].get<std::string>();
                        addError(j["error_text"].get<std::string>());
                    } catch (const std::exception& e) {
                        reslog(reslog::error) << "Error occurred but could not parse response: " << response << " (" << e.what() << ")";
                    }

                } else {
                    std::string iv;
                    try {
                        std::string decryptedResponse = Ripe::decryptAES(response, m_key, iv, true);
                        j = json::parse(decryptedResponse);
                        if (j.count("error_text") > 0) {
                            reslog(reslog::error) << j["error_text"].get<std::string>();
                            addError(j["error_text"].get<std::string>());
                        } else {
                            unsigned long now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::seconds(1);
                            Token t { j["token"].get<std::string>(), j["life"].get<int>(), now };
                            RESIDUE_LOCK_LOG("obtainToken");
                            std::lock_guard<std::recursive_mutex> lock(m_mutex);
                            if (m_tokens.find(loggerId) != m_tokens.end()) {
                                m_tokens.erase(loggerId);
                            }
                            m_tokens.insert(std::make_pair(loggerId, t));
                        }
                    } catch (const std::exception& e) {
                        reslog(reslog::error) << "Failed to read token response: " << e.what();
                    }
                }
            }
        });
    } catch (const ResidueException& e) {
        throw e;
    }
}

void Residue::loadConfiguration(const std::string& jsonFilename)
{
    std::ifstream fs(jsonFilename.c_str(), std::ios::in);
    if (!fs.is_open()) {
        throw ResidueException("File [" + jsonFilename + "] is not readable");
    }
    std::string confJson = std::string(std::istreambuf_iterator<char>(fs), std::istreambuf_iterator<char>());
    fs.close();
    Residue* r = &Residue::instance();
    json j;
    try {
        j = json::parse(confJson);
    } catch (const std::exception& e) {
        throw ResidueException("Unable to parse configuration file: " + std::string(e.what()));
    }
    if (j.count("url") == 0) {
        throw ResidueException("URL not specified in configuration");
    }
    std::string url = j["url"].get<std::string>();
    std::size_t pos = url.find_first_of(':');
    if (pos == std::string::npos) {
        throw ResidueException("URL should be in format of <host>:<port>");
    }
    r->m_host = url.substr(0, pos);
    std::string port = url.substr(pos + 1);
    r->m_port = atoi(port.c_str());
    if (r->m_port == 0) {
        throw ResidueException("Invalid port");
    }
    if (j.count("client_id") > 0 && j.count("client_private_key") > 0) {
        std::string privateKeyFile = j["client_private_key"].get<std::string>();
        if (j.count("client_key_secret") > 0) {
            std::string secretKey = j["client_key_secret"].get<std::string>();
            if (!secretKey.empty()) {
                r->m_rsaPrivateKeySecret = secretKey;
            }
        }
        std::ifstream fsPriv(privateKeyFile.c_str(), std::ios::in);
        if (fsPriv.is_open()) {
            setKnownClient(j["client_id"].get<std::string>(),
                    std::string(std::istreambuf_iterator<char>(fsPriv), std::istreambuf_iterator<char>()));
            fsPriv.close();
        } else {
            throw ResidueException("File [" + privateKeyFile + "] not readable");
        }
    }
    if (j.count("server_public_key") > 0) {
        std::string publicKeyFile = j["server_public_key"].get<std::string>();
        std::ifstream fsPub(publicKeyFile.c_str(), std::ios::in);
        if (fsPub.is_open()) {
            r->m_serverPublicKey = std::string(std::istreambuf_iterator<char>(fsPub), std::istreambuf_iterator<char>());
            fsPub.close();
        } else {
            throw ResidueException("File [" + publicKeyFile + "] not readable");
        }
    }
    if (j.count("application_id") > 0) {
        r->m_applicationId = j["application_id"].get<std::string>();
    }
    if (j.count("utc_time") > 0) {
        r->m_utc = j["utc_time"].get<bool>();
    }
    if (j.count("plain_request") > 0) {
        r->m_plainRequest = j["plain_request"].get<bool>();
    }
    if (j.count("time_offset") > 0) {
        r->m_timeOffset = j["time_offset"].get<int>();
    }
    if (j.count("rsa_key_size") > 0) {
        setKeySize(j["rsa_key_size"].get<std::size_t>());
    }
    if (j.count("dispatch_delay") > 0) {
        r->m_dispatchDelay = j["dispatch_delay"].get<int>();
    }
    if (j.count("main_thread_id") > 0) {
        setThreadName(j["main_thread_id"].get<std::string>());
    }
    if (j.count("access_codes") > 0) {
        AccessCodeMap map;
        for (auto& ac : j["access_codes"]) {
            try {
                std::string loggerId = ac["logger_id"].get<std::string>();
                std::string code = ac["code"].get<std::string>();
                map.insert(std::make_pair(loggerId, code));
            } catch (const std::exception&) {
                throw ResidueException("Invalid access_codes object. " + ac.dump());
            }
        }
        r->moveAccessCodeMap(std::move(map));
    }
    if (j.count("internal_logging_level") > 0) {
        Residue::s_internalLoggingLevel = j["internal_logging_level"].get<int>();
    }
}

void Residue::addError(const std::string& errorText) noexcept
{
    if (m_errors.size() > 10) {
        m_errors.erase(m_errors.begin(), m_errors.begin() + m_errors.size() - 10);
    }
    m_errors.push_back(errorText);
}

std::string Residue::version() noexcept
{
    return RESIDUE_SOVERSION;
}

std::string Residue::info() noexcept
{
    std::stringstream ss;
    ss << "Residue Client Library v" << version() << " based on Ripe v" << Ripe::version() << " and Easylogging++ v" << el::VersionInfo::version();
    return ss.str();
}
