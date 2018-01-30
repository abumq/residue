//
//  base16.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifdef RESIDUE_USE_MINE
#include "deps/mine/mine.h"
#else
#include "deps/ripe/Ripe.h"
#endif
#include "include/log.h"
#include "src/crypto/base16.h"

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
