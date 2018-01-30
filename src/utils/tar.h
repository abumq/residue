//
//  tar.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef Tar_h
#define Tar_h

#include <iostream>
#include "src/non-copyable.h"

namespace residue {

///
/// \brief Utility functions to create TAR archive
///
class Tar : NonCopyable
{
public:
    Tar(std::ostream& m_out);
    virtual ~Tar();
    void finish();
    void put(const char* filename, const std::string& s);
    void put(const char* filename, const char* content, std::size_t len);
    bool putFile(const char* filename, const char* nameInArchive);

private:
    struct Header
    {
        char name[100];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char checksum[8];
        char typeflag[1];
        char linkname[100];
        char magic[6];
        char version[2];
        char uname[32];
        char gname[32];
        char devmajor[8];
        char devminor[8];
        char prefix[155];
        char pad[12];
    };

    bool m_finished;
    std::ostream& m_out;

    void initialize(Header* header);
    void setChecksum(Header* header);
    void setSizeInArchive(Header* header, unsigned long fileSize);
    void setFilenameInArchive(Header* header, const char* filename);
    void addDelimiter(std::size_t len);
};
}

#endif /* Tar_h */