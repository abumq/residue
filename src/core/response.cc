//
//  response.cc
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

#include "core/response.h"

#include "net/session.h"

using namespace residue;

const std::unordered_map<unsigned short, std::string> Response::STANDARD_RESPONSES = {
    { static_cast<unsigned short>(Response::StatusCode::OK), "{\"r\":0}\r\n\r\n" },
    { static_cast<unsigned short>(Response::StatusCode::CONTINUE), "{\"r\":0}\r\n\r\n" },
    { static_cast<unsigned short>(Response::StatusCode::BAD_REQUEST), "{\"r\":1}\r\n\r\n" },
    { static_cast<unsigned short>(Response::StatusCode::INVALID_CLIENT), "{\"r\":2}\r\n\r\n" },
};
