#ifndef HttpClient_h
#define HttpClient_h

#include <string>
#include "src/static-base.h"

class HttpClient : StaticBase
{
public:
    static std::string fetchUrlContents(const std::string& url);
};

#endif /* HttpClient_h */