//
//  tar.cc
//  Residue
//
//  Copyright 2017-present Amrayn Web Services
//  https://amrayn.com
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

#include "utils/tar.h"

#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "logging/log.h"

using namespace residue;

Tar::Tar(std::ostream& out) :
    m_finished(false),
    m_out(out)
{
    if (sizeof(Header) != 512) {
        RLOG(ERROR) << "Invalid tar header size: " << sizeof(Header);
    }
}

Tar::~Tar()
{
    if (!m_finished) {
        RLOG(WARNING) << "Tar file was not finished.";
    }
}

void Tar::initialize(Header* header)
{
    std::memset(header, 0, sizeof(Header));
    std::strcpy(header->magic, "ustar");
    std::strcpy(header->version, " ");
    std::sprintf(header->mtime, "%011lo", time(NULL));
    std::sprintf(header->mode, "%07o", 0644);
    char* s = ::getlogin();
    if (s != nullptr) {
        std::snprintf(header->uname, 32, "%s", s);
    }
    std::sprintf(header->gname, "%s", "users");
}

void Tar::setChecksum(Header* header)
{
    unsigned int sum = 0;
    char* p = reinterpret_cast<char*>(header);
    char* q = p + sizeof(Header);
    while (p < header->checksum) {
        sum += *p++ & 0xff;
    }
    for (int i = 0; i < 8; ++i)  {
        sum += ' ';
        ++p;
    }
    while (p < q) {
        sum += *p++ & 0xff;
    }

    std::sprintf(header->checksum, "%06o", sum);
}

void Tar::setSizeInArchive(Header* header, unsigned long fileSize)
{
    std::sprintf(header->size, "%011llo", static_cast<long long unsigned int>(fileSize));
}

void Tar::setFilenameInArchive(Header* header, const char* filename)
{
    if (filename == nullptr || filename[0] == 0 || std::strlen(filename) >= 100) {
        RLOG(ERROR) << "invalid archive name [" << filename << "]";
        return;
    }
    std::snprintf(header->name,100, "%s", filename);
}

void Tar::addDelimiter(std::size_t len)
{
    char delim = '\0';
    while ((len % sizeof(Header)) != 0) {
        m_out.write(&delim, sizeof(char));
        ++len;
    }
}

void Tar::finish()
{
    m_finished = true;
    Header header;
    std::memset(&header, 0, sizeof(Header));
    m_out.write(reinterpret_cast<const char*>(&header), sizeof(Header));
    m_out.write(reinterpret_cast<const char*>(&header), sizeof(Header));
    m_out.flush();
}

void Tar::put(const char* filename, const std::string& s)
{
    put(filename, s.c_str(), s.size());
}

void Tar::put(const char* filename, const char* content, std::size_t len)
{
    Header header;
    initialize(&header);
    setFilenameInArchive(&header, filename);
    header.typeflag[0] = 0;
    setSizeInArchive(&header, len);
    setChecksum(&header);
    m_out.write(reinterpret_cast<const char*>(&header), sizeof(Header));
    m_out.write(content, len);
    addDelimiter(len);
}

bool Tar::putFile(const char* filename, const char* nameInArchive)
{
    char buff[BUFSIZ];
    std::FILE* in = std::fopen(filename,"rb");
    if (in == nullptr) {
        RLOG(ERROR) << "Cannot open " << filename << " " << std::strerror(errno);
        return false;
    }
    std::fseek(in, 0L, SEEK_END);
    long int len = std::ftell(in);
    std::fseek(in, 0L, SEEK_SET);

    Header header;
    initialize(&header);
    setFilenameInArchive(&header, nameInArchive);
    header.typeflag[0] = 0;
    setSizeInArchive(&header, len);
    setChecksum(&header);
    m_out.write(reinterpret_cast<const char*>(&header), sizeof(Header));
    std::size_t nRead = 0;
    while((nRead = std::fread(buff, sizeof(char), BUFSIZ, in)) > 0) {
        m_out.write(buff, nRead);
    }
    std::fclose(in);
    addDelimiter(len);
    return true;
}
