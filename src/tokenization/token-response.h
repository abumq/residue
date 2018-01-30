//
//  token-response.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef TokenResponse_hpp
#define TokenResponse_hpp

#include <string>
#include "src/core/response.h"

namespace residue {

class Configuration;
class Client;

///
/// \brief Response for TokenRequest
///
class TokenResponse final : public Response
{
public:
    TokenResponse(Response::StatusCode, const std::string&);
    TokenResponse(const std::string&, int life,
                  const std::string& loggerId);
    TokenResponse(bool);

    void serialize(std::string& output) const;

private:
    Response::StatusCode m_status;
    std::string m_errorText;
    std::string m_token;
    std::string m_loggerId;
    int m_life;
};
}
#endif /* TokenResponse_h */