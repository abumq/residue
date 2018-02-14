//
//  response.cc
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

#include "core/response.h"
#include "net/session.h"

using namespace residue;

const Response::StandardResponse Response::STANDARD_RESPONSES[] = {
    { Response::StatusCode::STATUS_OK, "{r:0}" + Session::PACKET_DELIMITER },
    { Response::StatusCode::CONTINUE, "{r:0}" + Session::PACKET_DELIMITER },
    { Response::StatusCode::BAD_REQUEST, "{r:1}" + Session::PACKET_DELIMITER },
};

void Response::serialize(JsonObject::Json& root,
                         std::string& output) const
{
    output = root.dump();
}
