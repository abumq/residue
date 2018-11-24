//
//  base16.h
//  Residue
//
//  Copyright 2017-present Zuhd Web Services
//  https://zuhd.org
//
//  Author: @abumusamq
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#ifndef Base16_h
#define Base16_h

#include <string>
#include "static-base.h"

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
