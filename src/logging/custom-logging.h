//
//  custom-logging.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include <chrono>
#include "include/log.h"

namespace std {
namespace chrono {
inline MAKE_LOGGABLE(std::chrono::seconds, d, os) {
    return os << d.count() << "s";
}
} // chrono
} // std