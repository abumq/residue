//
//  request.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Request_h
#define Request_h

#include <string>
#include "src/non-copyable.h"
#include "src/core/json-object.h"

namespace residue {

class Client;
class RequestHandler;
class Configuration;

///
/// \brief Base request class
/// \see AdminRequest
/// \see ConnectionRequest
/// \see TokenRequest
/// \see LogRequest
///
class Request : NonCopyable
{
public:
    enum StatusCode : unsigned short
    {
        STATUS_OK = 0,
        BAD_REQUEST = 1,
        CONTINUE = 0
    };

    Request(const Configuration* conf);
    virtual ~Request() = default;

    inline bool isValid() const
    {
        return m_isValid;
    }

    bool deserialize(std::string&& json);
    inline const JsonObject& jsonObject() const
    {
        return m_jsonObject;
    }

    inline Client* client() const
    {
        return m_client;
    }

    inline const std::string& errorText() const
    {
        return m_errorText;
    }

    inline const StatusCode& statusCode() const
    {
        return m_statusCode;
    }

    inline const std::string& ipAddr() const
    {
        return m_ipAddr;
    }

    inline const unsigned long& dateReceived() const
    {
        return m_dateReceived;
    }

    inline void setIpAddr(const std::string& ipAddr)
    {
        m_ipAddr = ipAddr;
    }

    inline void setDateReceived(const unsigned long& dateReceived)
    {
        m_dateReceived = dateReceived;
    }

    inline void setClient(Client* client)
    {
        m_client = client;
    }

    template <class Other>
    inline bool typeOf() const
    {
        return dynamic_cast<const Other*>(this) != nullptr;
    }

    virtual bool validateTimestamp() const;
protected:
    JsonObject m_jsonObject;
    bool m_isValid;

    Client* m_client;
    std::string m_errorText;
    StatusCode m_statusCode;
    std::string m_ipAddr;

    unsigned long m_timestamp;

    unsigned long m_dateReceived;

    const Configuration* m_configuration;

    friend class RequestHandler;
};
}

#endif /* Request_h */