//
//  residue-exception.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef ResidueException_h
#define ResidueException_h

#include <exception>

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
    ResidueException(const std::string& msg) : runtime_error(msg) {}

    virtual ~ResidueException() = default;
};

}
#endif /* ResidueException_h */