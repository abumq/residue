//
//  base16.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Base16_h
#define Base16_h

#include <string>
#include "src/static-base.h"

namespace residue {

///
/// \brief Base16 encoding wrappers
///
class Base16 final : StaticBase
{
public:

    static std::string decode(const std::string& encoded);
    static std::string encode(const std::string& raw);
};
}

#endif /* Base16_h */