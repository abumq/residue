//
//  Residue.h
//
//  Official C++ client library for Residue logging server
//
//  Copyright © 2017 Muflihun Labs. All rights reserved.
//
//  https://muflihun.com
//  https://muflihun.github.io/residue
//  https://github.com/muflihun
//

#ifndef Residue_h
#define Residue_h

#include <cstdint>
#include <ctime>
#include <atomic>
#include <deque>
#include <exception>
#include <mutex>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <easylogging++.h>

class ResidueDispatcher;

///
/// \brief Exception thrown by all the residue helper and internal methods
///
class ResidueException : public std::runtime_error
{
public:
    ///
    /// \brief Main constructor
    ///
    ResidueException(const std::string& msg) : runtime_error(msg) {}
};

///
/// \brief Crash handler provided by residue
///
void residueCrashHandler(int) noexcept;

///
/// \brief The Residue class provides helper methods to connect and interact to residue
/// server seamlessly.
///
class Residue {
public:
    ///
    /// \brief Server flags enum that are relavant to the client
    ///
    enum Flag : unsigned int {
        NONE = 0,
        ALLOW_UNKNOWN_LOGGERS = 1,
        CHECK_TOKENS = 2,
        ALLOW_DEFAULT_ACCESS_CODE = 4,
        ALLOW_PLAIN_LOG_REQUEST = 8,
        ALLOW_BULK_LOG_REQUEST = 16,
        COMPRESSION = 256,
    };

    enum class InternalLoggingLevel : std::int8_t {
        crazy = 0,
        debug = 1,
        info = 2,
        warning = 3,
        error = 4,
        none = 5
    };

#ifdef RESIDUE_PROFILING
    unsigned long queue_ms;
    unsigned long dispatch_ms;
    int c_total;
    int c_dispatched;
#endif
    ///
    /// \brief Map of logger_id and corresponding access code to send log message to the server
    ///
    using AccessCodeMap = std::unordered_map<std::string, std::string>;

    ///
    /// \brief Default key size for RSA
    /// \see setKeySize
    ///
    static const std::size_t DEFAULT_KEY_SIZE;

    ///
    /// \brief Default port 8777 of residue
    ///
    static const int DEFAULT_PORT;

    ///
    /// \brief 127.0.0.1
    ///
    static const std::string LOCALHOST;

    ///
    /// \brief Threshold (seconds) to check against client age and whether
    /// we should send touch request or not
    ///
    static const unsigned int TOUCH_THRESHOLD;

    ///
    /// \brief Default access code value if allowed
    ///
    static const std::string DEFAULT_ACCESS_CODE;

    ///
    /// \brief See setInternalLoggingLevel(InternalLoggingLevel)
    ///
    static volatile int s_internalLoggingLevel;

    ///
    /// \brief Singleton instance
    ///
    static inline Residue& instance() noexcept
    {
        static Residue s_instance;
        return s_instance;
    }

    virtual ~Residue() noexcept;

    Residue(Residue&&) = delete;
    Residue(const Residue&) = delete;
    void operator=(const Residue&) = delete;

    ///
    /// \brief Version of Residue library
    ///
    static std::string version() noexcept;

    ///
    /// \brief Version of Residue library and dependencies
    ///
    static std::string info() noexcept;

    ///
    /// \brief Connect to residue on specified host
    /// \param host Host where residue server is running
    /// \param port Residue server port (Try Residue::DEFAULT_PORT)
    /// \param accessCodeMap Map of logger_id and access_codes to access corresponding logger.
    /// \throws exception When connection fails
    ///
    static inline void connect(const std::string& host, int port, AccessCodeMap* accessCodeMap)
    {
        Residue::instance().connect_(host, port, accessCodeMap);
    }

    ///
    /// \brief Connect to residue on specified host and port using existing access code map
    /// \param host Host where residue server is running
    /// \param port Residue server port (Try Residue::DEFAULT_PORT)
    /// \see moveAccessCodeMap(AccessCodeMap&&)
    /// \throws exception When connection fails
    ///
    static inline void connect(const std::string& host, int port)
    {
        Residue::instance().connect_(host, port, Residue::instance().m_accessCodeMap);
    }

    ///
    /// \brief Connect to residue on localhost
    /// \see connect(const std::string&, int, AccessCodeMap*)
    /// \throws exception When connection fails
    ///
    static inline void connect(int port, AccessCodeMap* accessCodeMap)
    {
        Residue::instance().connect_(Residue::LOCALHOST, port, accessCodeMap);
    }

    ///
    /// \brief Connect to residue using default port
    /// \see connect(const std::string&, int, AccessCodeMap*)
    /// \throws exception When connection fails
    ///
    static inline void connect(const std::string& host, AccessCodeMap* accessCodeMap)
    {
        Residue::instance().connect_(host, Residue::DEFAULT_PORT, accessCodeMap);
    }

    ///
    /// \brief Connect to residue on localhost using default port
    /// \see connect(const std::string&, int, AccessCodeMap*)
    /// \throws exception When connection fails
    ///
    static inline void connect(AccessCodeMap* accessCodeMap)
    {
        Residue::instance().connect_(Residue::LOCALHOST, Residue::DEFAULT_PORT, accessCodeMap);
    }

    ///
    /// \brief Connect to residue on localhost using default port and previously provided access code map.
    /// This is more useful when you have client ID so you won't have varying client IDs
    /// \see connect(const std::string&, int, AccessCodeMap*)
    /// \see setParams(const std::string&, const std::string&)
    /// \throws exception When connection fails
    ///
    static inline void connect()
    {
        Residue::instance().connect_(Residue::LOCALHOST, Residue::DEFAULT_PORT, Residue::instance().m_accessCodeMap);
    }

    ///
    /// \brief Reconnects residue server using new or initially provided parameters. (Disconnect then connect)
    /// \param host If left empty, existing value (initially provided) will be used
    /// \param port If set to -1, existing value (initially provided) will be used
    /// \param accessCodeMap If null provided, existing value (initially provided) will be used
    ///
    static inline void reconnect(const std::string& host = "", int port = -1, AccessCodeMap* accessCodeMap = nullptr)
    {
        Residue::disconnect();
        Residue::connect(host.empty() ? Residue::instance().m_host : host,
                         port == -1 ? Residue::instance().m_port : port,
                         accessCodeMap == nullptr ? Residue::instance().m_accessCodeMap : accessCodeMap);
    }

    ///
    /// \brief Moves access code map in to Residue own memory (rather than keeping it in application scope)
    ///
    static inline void moveAccessCodeMap(AccessCodeMap&& accessCodeMap) noexcept
    {
        Residue::instance().m_accessCodeMapByVal = std::move(accessCodeMap);
        Residue::instance().m_accessCodeMap = &Residue::instance().m_accessCodeMapByVal;
    }

    ///
    /// \brief Check connection to the server
    /// \return Whether connected to the server or not
    ///
    static inline bool connected() noexcept
    {
        return Residue::instance().m_connected;
    }

    ///
    /// \brief Returns current client ID. Returns empty if not connected
    ///
    static inline const std::string& clientId() noexcept
    {
        return Residue::instance().m_clientId;
    }

    ///
    /// \brief Check connection to the server
    /// \return Whether connection to the server is in progress
    ///
    static inline bool connecting() noexcept
    {
        return Residue::instance().m_connecting;
    }

    ///
    /// \brief If connected this identifies maximum
    /// number of items in bulk that server accepts
    ///
    static inline unsigned int maxBulkSize() noexcept
    {
        return Residue::instance().m_maxBulkSize;
    }

    ///
    /// \brief If connected this identifies server flags.
    ///
    static inline unsigned int serverFlags() noexcept
    {
        return Residue::instance().m_serverFlags;
    }

    ///
    /// \brief Safely disconnects from the server. This will also call wait()
    ///
    /// You should always call this before ending your application.
    ///
    static void disconnect() noexcept;

    ///
    /// \brief Wait to dispatch all the log requests to the server.
    ///
    static inline void wait() noexcept
    {
        if (Residue::instance().m_dispatcher->joinable()) {
            Residue::instance().m_dispatcher->join();
        }
    }

    ///
    /// \brief Application ID is what gets passed on to the server for %app format specifier
    ///
    static inline void setApplicationId(const std::string& id) noexcept
    {
        Residue::instance().m_applicationId = id;
    }

    ///
    /// \brief enableBulkDispatch turns on bulk dispatch.
    /// \note MAKE SURE SERVER SUPPORTS BULK LOG REQUESTS
    /// \throws exception if server does not allow bulk requests. (can only determine when connected)
    ///
    static inline void enableBulkDispatch()
    {
        if (connected() && !hasFlag(Flag::ALLOW_BULK_LOG_REQUEST)) {
            throw std::invalid_argument("Bulk log requests not allowed by this server");
        }
        Residue::instance().m_bulkDispatch = true;
    }

    ///
    /// \brief disableBulkDispatch turns off bulk dispatch.
    ///
    static inline void disableBulkDispatch() noexcept
    {
        Residue::instance().m_bulkDispatch = false;
    }

    ///
    /// \brief Send UTC time to the server. By default local time is sent
    ///
    static inline void enableUtc() noexcept
    {
        Residue::instance().m_utc = true;
    }

    ///
    /// \brief Send Local time to the server. By default local time is sent
    ///
    static inline void disableUtc() noexcept
    {
        Residue::instance().m_utc = false;
    }

    ///
    /// \brief Offset in seconds for log time. [Default: 0]
    ///
    static inline void setTimeOffset(int offset) noexcept
    {
        Residue::instance().m_timeOffset = offset;
    }

    ///
    /// \brief Delay between dispatching log messages (in milliseconds) [Default: 1ms]
    ///
    /// \details Setting correct delay is very important depending on your application and needs.
    /// Some applications that run for long (e.g, GUI application) may need longer delays
    /// between collecting log message and dispatching them to the server. This is very helpful
    /// for bulk requests in combination with compression enabled servers. When you collect
    /// many messages before they are dispatched to the server and you have bulk request and
    /// compression enabled, there are more changes of similar strings, meaning smaller packets.
    ///
    /// For example, if you have dispatch delay set to 10000 (10 seconds) and you have
    /// bulk requests (size 20) and compression enabled, when you log messages
    /// (using any method, e.g, LOG(INFO)...) it is going to hold the message in local memory
    /// for 10 seconds. Let's say within 10 seconds you logged 100 times, in this situation there
    /// are high chances you will have duplicate strings (for example lots of ":" or alphabets (e.g, "a")
    /// hence the compression will be much more useful.
    ///
    static inline void setDispatchDelay(unsigned int delay) noexcept
    {
        Residue::instance().m_dispatchDelay = delay;
    }

    ///
    /// \brief setBulkSize number of log messages to be bulked together
    /// \note This depends on what server accepts. The configuration value on the server is <code>max_items_in_bulk</code>.
    /// Please contact your log server hosting to get this number. Failing to select correct number can fail.
    /// \throws ResidueException if connected and specified bulk size exceeds maximum allowed
    ///
    static inline void setBulkSize(unsigned int bulkSize)
    {
        if (connected() && bulkSize > maxBulkSize()) {
            throw ResidueException("Invalid bulk dispatch size. Maximum allowed: " + std::to_string(maxBulkSize()));
        }
        Residue::instance().m_bulkSize = bulkSize;
    }

    ///
    /// \brief Enables automatic setting of bulk parameters depending on
    /// what's most efficient. It essentially enables bulk if server supports
    /// it and sets bulk size and dispatch delay accordingly.
    ///
    /// \note You need re-connect using <pre>connect()</pre> helper method
    /// \note By default it is enabled
    ///
    static inline void enableAutoBulkParams() noexcept
    {
        Residue::instance().m_autoBulkParams = true;
    }

    ///
    /// \brief Disables automatic setting of bulk params
    /// \note It does not reset parameters back to what they were, meaning
    /// this only implies it does not change any parameters when reconnected
    /// \see enableAutoBulkParams()
    ///
    static inline void disableAutoBulkParams() noexcept
    {
        Residue::instance().m_autoBulkParams = false;
    }

    ///
    /// \brief Size of RSA key (in bits) when connecting.
    ///
    /// \details RSA key is generated on the fly when client is unknown to the server.
    ///
    /// You can choose to speed up connection process by understanding 3 following points and acting accordingly;
    ///  * Default value is 2048-bit which takes ~150ms to generate the key, if you increase it to 4096-bit
    /// it takes ~3s to generate this key. This means connection will be 2850 ms slow but is more secure. This situation is
    /// useful for big applications that can take this long, e.g, gui application
    ///  * Having pre-defined key helps speed up connection process by saving overhead of key generation with each connection.
    ///
    /// \param keySize Size of key in bits, i.e, 2048, 4096, ...
    /// \see setKnownClient
    /// \throws ResidueException if size is not one of 2048, 4096 or 8192
    ///
    static inline void setKeySize(std::size_t keySize)
    {
        if (keySize != 2048 && keySize != 2048 * 2 && keySize != 2048 * 4) {
            throw ResidueException("Invalid key size. Please select 2048, 4096 or 8192");
        }
        Residue::instance().m_keySize = keySize;
    }

    ///
    /// \see setApplicationArgs(int, const char*)
    ///
    static inline void setApplicationArgs(int argc, char** argv)
    {
        START_EASYLOGGINGPP(argc, argv);
    }

    ///
    /// \brief Wrapper for START_EASYLOGGINGPP.
    /// \details This is recommended for all the applications using Residue in order to send verbose logs to the server.
    /// \see https://github.com/muflihun/easyloggingpp#setting-application-arguments
    ///
    static inline void setApplicationArgs(int argc, const char** argv)
    {
        START_EASYLOGGINGPP(argc, argv);
    }

    ///
    /// \brief If server knows you already you can let us know and we will use this identity.
    /// \note YOU NEED TO RUN CONNECT() AFTER THIS
    /// \param clientId what server knows you as
    /// \param privateKeyPem Contents of your RSA private key. THIS IS NOT PATH TO THE FILE RATHER IT'S CONTENTS OF THE FILE
    /// \see connect
    /// \throws ResidueException if you specified client ID but private key is null
    ///
    static inline void setKnownClient(const std::string& clientId, const std::string& privateKeyPem)
    {
        Residue::instance().m_clientId = clientId;
        Residue::instance().m_rsaPrivateKey = privateKeyPem;
        if (!clientId.empty() && privateKeyPem.empty()) {
            throw ResidueException("Please provide private key for known client.");
        }
        Residue::instance().m_knownClient = !clientId.empty() && !privateKeyPem.empty();
    }

    ///
    /// \brief Sets server public key for encrypted connection.
    /// \note The bigger the server key, the slower server will respond (but it's more secure)
    ///
    static inline void setServerPublicKey(const std::string& publicKeyPem)
    {
        Residue::instance().m_serverPublicKey = publicKeyPem;
    }

    ///
    /// \brief Sets logging level for residue client library internal logging.
    /// \param level CRAZY = 0, DEBUG = 1, INFO = 2, WARNING = 3, ERROR = 4, NONE = 5 (default)
    ///
    /// E.g, for ERROR and WARNING = use level 3
    /// for just ERROR = use level 4
    ///
    static inline void setInternalLoggingLevel(int level)
    {
        Residue::s_internalLoggingLevel = level;
    }

    ///
    /// \brief Helper function to set logging level for debugging/info
    /// \see setInternalLoggingLevel(int)
    ///
    static inline void setInternalLoggingLevel(InternalLoggingLevel level)
    {
        setInternalLoggingLevel(static_cast<int>(level));
    }

    ///
    /// \brief Checks for server flag and returns true if flag set otherwise not.
    ///
    static inline bool hasFlag(Flag flag) noexcept
    {
        return serverFlags() != 0 && (serverFlags() & flag) != 0;
    }

    ///
    /// \brief Send plain log request when possible (i.e, when server allows it)
    ///
    static inline void enablePlainRequest() noexcept
    {
        Residue::instance().m_plainRequest = true;
    }

    ///
    /// \brief Disable plain log request
    /// \see enablePlainRequest()
    ///
    static inline void disablePlainRequest() noexcept
    {
        Residue::instance().m_plainRequest = false;
    }

    ///
    /// \brief Sets current thread name
    ///
    static inline void setThreadName(const std::string& threadName) noexcept
    {
        el::Helpers::setThreadName(threadName);
    }

    ///
    /// \brief Enables crash handler
    /// \param loggerId Logger ID for handling crash
    ///
    static inline void enableCrashHandler(const std::string& loggerId = el::base::consts::kDefaultLoggerId)
    {
#if defined(ELPP_FEATURE_ALL) || defined(ELPP_FEATURE_CRASH_LOG)
        Residue::instance().m_crashHandlerLoggerId = loggerId;
        el::Helpers::setCrashHandler(residueCrashHandler);
#endif
    }

    ///
    /// \brief Useful for debugging the issue.
    /// Contains last 10 errors from server (i.e, error_text in response)
    /// Last error is the latest, second last is second latest, ...
    ///
    static inline const std::vector<std::string>& errors() noexcept
    {
        return Residue::instance().m_errors;
    }

    ///
    /// \brief Loads all the configurations from JSON file.
    /// \details  A JSON file should look similar to. Only <pre>url</pre> is required, rest of them are optional
    /// <pre>
    /// {
    ///     "url": "localhost:8777",
    ///     "access_codes": [
    ///         {
    ///             "logger_id": "sample-app",
    ///             "code": "134193F1A993FA6A4FE"
    ///         }
    ///     ],
    ///     "application_id": "com.muflihun.sampleapp",
    ///     "rsa_key_size": 2048,
    ///     "plain_request": false,
    ///     "utc_time": false,
    ///     "time_offset": -3600,
    ///     "dispatch_delay": 1,
    ///     "main_thread_id": "main_thread",
    ///     "client_id": "my_client",
    ///     "client_private_key": "client-private.pem",
    ///     "client_key_secret": "",
    ///     "server_public_key": "server-public.pem",
    ///     "internal_logging_level": 2
    /// }
    /// </pre>
    /// \param jsonFilename Path to configuration JSON file
    ///
    static void loadConfiguration(const std::string& jsonFilename);

    ///
    /// \brief Crash handler logger. Not for public use.
    ///
    inline std::string crashHandlerLoggerId() const noexcept
    {
        return m_crashHandlerLoggerId;
    }

    ///
    /// \brief Gets timestamp for replay attack prevention. This function
    /// always uses unix-time for comparison.
    ///
    unsigned long getTimestamp() const noexcept
    {
        std::time_t seconds;
        std::time(&seconds);
        return seconds;
    }

private:

    // defs and structs

    using RequestTuple = std::tuple<
        unsigned long, // datetime
        std::string, // thread_id / name
        std::string, // logger_id
        el::base::type::string_t, // msg
        std::string, // file
        el::base::type::LineNumber, // line
        std::string, // func
        unsigned int, // level
        el::base::type::VerboseLevel // verbose level
    >;

    struct Token
    {
        std::string token;
        int life;
        unsigned long dateCreated;
    };

    Residue() noexcept;

    // token
    void obtainToken(const std::string& loggerId, const std::string& accessCode);
    void obtainToken(const std::string& loggerId);
    void obtainTokens();
    bool hasToken(const std::string& loggerId) const noexcept;
    std::string getToken(const std::string& loggerId) const;

    // client
    bool isClientValid() const noexcept;
    bool shouldTouch() const noexcept;
    void touch() noexcept;
    void healthCheck() noexcept;

    // connect
    void connect_(const std::string& host, int port, AccessCodeMap* accessCodeMap);
    void disconnect_() noexcept;
    void reset();
    void onConnect() noexcept;

    // request
    void addToQueue(RequestTuple&&) noexcept;
    void dispatch();
    std::string requestToJson(RequestTuple&& request);

    void addError(const std::string& errorText) noexcept;

    friend class ResidueDispatcher;

    // private members

    std::string m_host;
    int m_port;

    AccessCodeMap* m_accessCodeMap;
    AccessCodeMap m_accessCodeMapByVal;
    std::unordered_map<std::string, Token> m_tokens;

    std::atomic<bool> m_connected;
    std::atomic<bool> m_connecting;
    std::atomic<bool> m_disconnected; // user called disconnect()

    std::atomic<bool> m_bulkDispatch;
    unsigned int m_bulkSize;
    unsigned int m_maxBulkSize;

    std::atomic<bool> m_plainRequest;

    std::string m_rsaPrivateKey;
    std::string m_rsaPrivateKeySecret;
    std::string m_rsaPublicKey;
    std::size_t m_keySize;
    std::string m_clientId;
    std::string m_key;
    std::string m_serverPublicKey;
    unsigned int m_age;
    unsigned long m_dateCreated;
    int m_loggingPort;
    int m_tokenPort;
    unsigned int m_serverFlags;
    std::atomic<bool> m_utc;
    std::atomic<int> m_timeOffset;
    std::atomic<unsigned int> m_dispatchDelay;

    std::deque<RequestTuple> m_requests;

    std::recursive_mutex m_mutex;
    std::thread* m_dispatcher;
    std::atomic<bool> m_running;
    std::atomic<bool> m_autoBulkParams;

    std::string m_applicationId;
    bool m_knownClient;

    std::string m_crashHandlerLoggerId;

    std::vector<std::string> m_errors;
    std::mutex m_errorsMutex;
};

#endif /* Residue_h */
