//
//  json-doc.h
//  Residue
//
//  Copyright 2017-present Muflihun Labs
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

#ifndef JsonDoc_h
#define JsonDoc_h

#include <sstream>
#include "gason/gason.h"
#include "gason/jsonbuilder.h"

namespace residue {

using JsonBuilder = gason::JSonBuilder;

struct JsonDoc
{
    using Value = gason::JsonValue;
    using Status = gason::JsonParseStatus;

    Status status;
    Value val;

    JsonDoc()
    {
        status = gason::JsonParseStatus::JSON_PARSE_ALLOCATION_FAILURE;
    }

    void parse(const std::string& jstr);

    std::string dump() const;
    static void dump(Value o, std::stringstream& ss);
    static void dumpStr(const char* s, std::stringstream& ss);

    template <typename T>
    T get(const char* key, const T&) const
    {
        return val(key);
    }
private:
    gason::JsonAllocator alloc;
};
}

#endif /* JsonDoc_h */
