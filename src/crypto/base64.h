//
//  base64.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Base64_h
#define Base64_h

#include <string>
#include "src/static-base.h"

namespace residue {

///
/// \brief Base64 encoding wrappers
///
class Base64 final : StaticBase
{
public:

    static std::string decode(const std::string& encoded);
    static std::string encode(const std::string& raw);
};
}

#endif /* Base64_h */