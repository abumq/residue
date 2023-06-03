//
//  zlib.h
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
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

#ifndef ZLib_h
#define ZLib_h

#include <string>
#include "static-base.h"

namespace residue {

///
/// \brief ZLib wrappers
///
class ZLib final : StaticBase
{
public:

    ///
    /// \brief Deflate
    ///
    static std::string compress(const std::string& data);

    ///
    /// \brief Inflate
    ///
    static std::string decompress(const std::string& data);

    ///
    /// \brief Compress file (Deflate)
    ///
    static bool compressFile(const std::string& gzoutFilename, const std::string& inputFile);

};
}

#endif /* ZLib_h */
