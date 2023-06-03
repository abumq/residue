//
//  residue-exception.h
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

#ifndef ResidueException_h
#define ResidueException_h

#include <exception>
#include <stdexcept>

namespace residue {

///
/// \brief Exception thrown by all the residue helper and internal methods
///
class ResidueException : public std::runtime_error
{
public:
    ///
    /// \brief Main constructor
    ///
    explicit ResidueException(const std::string& msg) : runtime_error(msg) {}

    virtual ~ResidueException() = default;
};

}
#endif /* ResidueException_h */
