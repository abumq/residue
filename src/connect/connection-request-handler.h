//
//  connection-request-handler.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef ConnectionRequestHandler_h
#define ConnectionRequestHandler_h

#include "src/core/request-handler.h"

namespace residue {
class ConnectionRequest;

///
/// \brief Handle incoming ConnectRequest
/// \see ConnectionRequest
///
class ConnectionRequestHandler : public RequestHandler
{
public:
    explicit ConnectionRequestHandler(Registry*);
    virtual void handle(RawRequest&&);
private:
    void connect(ConnectionRequest*, bool isKnownClient) const;
    void acknowledge(const ConnectionRequest*) const;
    void touch(const ConnectionRequest*) const;
};
}

#endif /* ConnectionRequestHandler_h */