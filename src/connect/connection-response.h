//
//  connection-response.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef ConnectionResponse_h
#define ConnectionResponse_h

#include <string>
#include "src/core/response.h"
#include "src/connect/connection-request-handler.h"

namespace residue {

class Configuration;
class Client;
class Configuration;

///
/// \brief Response for connect request
/// \see ConnectionRequest
///
class ConnectionResponse final : public Response
{
public:
    explicit ConnectionResponse(Response::StatusCode, const std::string&);
    explicit ConnectionResponse(const Client*,
                                const Configuration* c = nullptr);
    void serialize(std::string& output) const;

    inline void setLoggingPort(unsigned int loggingPort)
    {
        m_loggingPort = loggingPort;
    }

    inline void setTokenPort(unsigned int tokenPort)
    {
        m_tokenPort = tokenPort;
    }

private:
    StatusCode m_status;
    std::string m_errorText;
    const Configuration* m_configuration;

    int m_tokenPort;
    int m_loggingPort;
    std::string m_key;
    std::string m_clientId;
    unsigned int m_clientAge;
    unsigned long m_clientDateCreated;
    bool m_isAcknowledged;

    friend class ConnectionRequestHandler;
};
}

#endif /* ConnectionResponse_h */