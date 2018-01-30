//
//  connection-request.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef ConnectionRequest_h
#define ConnectionRequest_h

#include <string>
#include "src/core/request.h"

namespace residue {

///
/// \brief Request entity to connect to the server. This request can be of following types:
/// Connect, Disconnect, Acknowledge, Touch
///
class ConnectionRequest final : public Request
{
public:
    enum class Type : unsigned short
    {
        UNKNOWN = 0,
        CONNECT = 1,
        ACKNOWLEDGE = 2,
        TOUCH = 3,
    };

    ConnectionRequest(const Configuration* conf);

    inline const std::string& clientId() const
    {
        return m_clientId;
    }

    inline void setKeySize(unsigned int keySize)
    {
        m_keySize = keySize;
    }

    inline void setClientId(const std::string& newClientId)
    {
        m_clientId = newClientId;
    }

    inline void setRsaPublicKey(const std::string& rsaPublicKey)
    {
        m_rsaPublicKey = rsaPublicKey;
    }

    inline const std::string& rsaPublicKey() const
    {
        return m_rsaPublicKey;
    }

    inline Type type() const
    {
        return m_type;
    }

    inline unsigned int keySize() const
    {
        return m_keySize;
    }

    bool deserialize(std::string&& json);
private:

    std::string m_clientId;
    std::string m_rsaPublicKey;
    unsigned int m_keySize;
    Type m_type;
};
}
#endif /* ConnectionRequest_h */