//
//  asio.h
//  Residue
//
//  Copyright 2017-present Zuhd Web Services
//  https://zuhd.org
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

#ifndef RESIDUE_ASIO_H
#define RESIDUE_ASIO_H

#ifdef RESIDUE_BOOST
#   include <boost/asio.hpp>
namespace net = boost::asio;
namespace residue {
    using error_code = boost::system::error_code;
}
#else
#   include <system_error>
#   include "asio.hpp"
namespace net = asio;
namespace residue {
    using error_code = std::error_code;
}
#endif // RESIDUE_BOOST

#endif /* RESIDUE_ASIO_H */
