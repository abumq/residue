//
//  response.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Response_h
#define Response_h

#include <string>
#include "src/non-copyable.h"
#include "src/core/json-object.h"

namespace residue {

///
/// \brief Simple response
///
class Response : NonCopyable
{
public:

    enum StatusCode : unsigned short
    {
        STATUS_OK = 0,
        BAD_REQUEST = 1,
        CONTINUE = 0
    };

    explicit Response() = default;
    virtual ~Response() = default;

    void serialize(JsonObject::Json& root,
                   std::string& output) const;
};
}
#endif /* Response_h */