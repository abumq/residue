//
//  zlib.cc
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
#include "src/crypto/zlib.h"

using namespace residue;

std::string ZLib::compress(const std::string& data)
{
#ifdef RESIDUE_USE_MINE
    try {
        return mine::ZLib::compressString(data);
    } catch (const std::exception& e) {
        std::cerr << "Failed to compress zlib " << data << ", e=" << e.what() << std::endl;
    }
    return "ERROR ZLIB COMPRESS";
#else
    return Ripe::compressString(data);
#endif
}

std::string ZLib::decompress(const std::string& data)
{
#ifdef RESIDUE_USE_MINE
    try {
        return mine::ZLib::decompressString(data);
    } catch (const std::exception& e) {
        RLOG(ERROR) << "Failed to decompress zlib " << data << ", e=" << e.what();
    }
    return "ERROR ZLIB DECOMRESS";
#else
    return Ripe::decompressString(data);
#endif
}

bool ZLib::compressFile(const std::string& gzoutFilename, const std::string& inputFile)
{
#ifdef RESIDUE_USE_MINE
    try {
        return mine::ZLib::compressFile(gzoutFilename, inputFile);
    } catch (const std::exception& e) {
        RLOG(ERROR) << "Failed to compress file with zlib " << inputFile << ", e=" << e.what();
    }
    return false;
#else
    return Ripe::compressFile(gzoutFilename, inputFile);
#endif
}
