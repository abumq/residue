//
//  admin-request-handler.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef AdminRequestHandler_h
#define AdminRequestHandler_h

#include "src/core/request-handler.h"

namespace residue {

class CommandHandler;
///
/// \brief Handles incoming AdminRequest
/// \see AdminRequest
///
class AdminRequestHandler : public RequestHandler
{
public:
    explicit AdminRequestHandler(Registry*, CommandHandler*);
    virtual void handle(RawRequest&&);
private:
    CommandHandler* m_commandHandler;

    void respond(const std::string&) const;
};
}

#endif /* AdminRequestHandler_h */