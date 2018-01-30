//
//  user-log-builder.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef UserLogBuilder_h
#define UserLogBuilder_h

#include "include/log.h"
#include "src/non-copyable.h"

namespace residue {

class LogRequest;

///
/// \brief Custom log builder for Residue
///
class UserLogBuilder final : public el::LogBuilder, NonCopyable
{
public:
    explicit UserLogBuilder();
    virtual el::base::type::string_t build(const el::LogMessage* logMessage,
                                   bool appendNewLine) const override;

    inline const LogRequest* request() const
    {
        return m_request;
    }

    inline void setRequest(const LogRequest* request)
    {
        m_request = request;
    }
private:
    const LogRequest* m_request;
};
}

#endif /* UserLogBuilder_h */