//
//  token-request.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef TokenRequest_h
#define TokenRequest_h

#include <string>
#include "src/core/request.h"

namespace residue {

///
/// \brief Incoming token request
///
class TokenRequest final : public Request
{
public:
    TokenRequest(const Configuration* conf);

    inline const std::string& loggerId() const
    {
        return m_loggerId;
    }

    inline const std::string& token() const
    {
        return m_token;
    }

    inline const std::string& accessCode() const
    {
        return m_accessCode;
    }

    inline bool isValid() const
    {
        return m_isValid;
    }

    bool deserialize(std::string&& json);
private:

    std::string m_loggerId;
    std::string m_accessCode;
    std::string m_token;

    bool m_isValid;
};
}
#endif /* TokenRequest_h */