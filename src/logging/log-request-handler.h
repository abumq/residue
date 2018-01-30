//
//  log-request-handler.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef LogRequestHandler_h
#define LogRequestHandler_h

#include <atomic>
#include <vector>
#include <thread>
#include <string>
#include <tuple>
#include "src/core/request-handler.h"
#include "src/logging/user-log-builder.h"
#include "src/logging/logging-queue.h"

namespace el {
    class LogBuilder;
    class LogMessage;
}

namespace residue {

class LogRequest;
class Configuration;

///
/// \brief Request handler for LogRequest
/// \see LogRequest
///
class LogRequestHandler : public RequestHandler
{
public:
    explicit LogRequestHandler(Registry*,
                               el::LogBuilder*);
    virtual ~LogRequestHandler();

    ///
    /// \breif Start handling client's requests
    ///
    void start();
    bool isRequestAllowed(const LogRequest*) const;

    virtual void handle(RawRequest&&);

    // Format specifiers
    inline std::string getClientId(const el::LogMessage*)
    {
        return m_userLogBuilder->request() != nullptr ? m_userLogBuilder->request()->clientId() : "";
    }

    inline std::string getIpAddr(const el::LogMessage*)
    {
        return m_userLogBuilder->request() != nullptr ? m_userLogBuilder->request()->ipAddr() : "";
    }
private:

    ////
    /// \brief Dispatches the request after temp configurating some elements of easylogging++
    ///
    void dispatch(const LogRequest* request);

    ///
    /// \brief Parses raw request and pushes it to m_requests ideally
    /// in separate thread/s (m_backgroundWorkers)
    ///
    void processRequestQueue();
    bool processRequest(LogRequest*, Client** client, bool forceCheck = false);

    bool isValidToken(const LogRequest*) const;

    UserLogBuilder* m_userLogBuilder;

    std::atomic<bool> m_stopped;

    LoggingQueue m_queue;

    std::vector<std::thread> m_backgroundWorkers;
};
}
#endif /* LogRequestHandler_h */