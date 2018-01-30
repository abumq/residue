//
//  token-request-handler.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef TokenRequestHandler_h
#define TokenRequestHandler_h

#include "src/core/request-handler.h"

namespace residue {

///
/// \brief Handles incoming token request
/// \see TokenRequest
///
class TokenRequestHandler final : public RequestHandler
{
public:
    explicit TokenRequestHandler(Registry*);
    virtual void handle(RawRequest&&);
};
}

#endif /* TokenRequestHandler_h */