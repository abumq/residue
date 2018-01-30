//
//  response.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/core/response.h"

using namespace residue;

void Response::serialize(JsonObject::Json& root,
                         std::string& output) const
{
    output = root.dump();
}