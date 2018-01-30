//
//  base64.cc
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
#include "src/crypto/base64.h"

using namespace residue;

std::string Base64::decode(const std::string& encoded)
{
#ifdef RESIDUE_USE_MINE
    try {
        return mine::Base64::decode(encoded);
    } catch (const std::exception& e) {
        // this can happen during initialization (static initialization)
        RLOG(ERROR) << "Failed to decode b64 " << encoded << ", e=" << e.what();
    }
    return "ERROR B64 DECODE";
#else
    return Ripe::base64Decode(encoded);
#endif
}

std::string Base64::encode(const std::string& raw)
{
#ifdef RESIDUE_USE_MINE
    try {
        return mine::Base64::encode(raw);
    } catch (const std::exception& e) {
        RLOG(ERROR) << "Failed to decode b64 " << raw << ", e=" << e.what();
    }
    return "ERROR B64 DECODE";
#else
    return Ripe::base64Encode(raw);
#endif
}
