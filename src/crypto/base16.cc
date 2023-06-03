//
//  base16.cc
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

#include "crypto/base16.h"

#include "logging/log.h"

#ifdef RESIDUE_USE_MINE
#   include "mine/mine.h"
#else
#   include "ripe/Ripe.h"
#endif

using namespace residue;

std::string Base16::decode(const std::string& encoded)
{
#ifdef RESIDUE_USE_MINE
    try {
        return mine::Base16::decode(encoded);
    } catch (const std::exception& e) {
        RLOG(ERROR) << "Failed to decode b16 " << encoded << ", e: " << e.what();
    }
    return "ERROR B16 DECODE";
#else
    return Ripe::hexToString(encoded);
#endif
}

std::string Base16::encode(const std::string& raw)
{
#ifdef RESIDUE_USE_MINE
    try {
        return mine::Base16::encode(raw);
    } catch (const std::exception& e) {
        RLOG(ERROR) << "Failed to decode b16 " << raw << ", e: " << e.what();
    }
    return "ERROR B16 DECODE";
#else
    return Ripe::stringToHex(raw);
#endif
}
