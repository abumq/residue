#ifndef LicenseManagerKeys_h
#define LicenseManagerKeys_h

#include <string>
#include <vector>
#include "src/non-copyable.h"
#include "src/licensing/issuing-authority.h"

namespace residue {

class LicenseManagerKeys final : NonCopyable
{
public:
    static const int LICENSE_MANAGER_SIGNATURE_KEY[];

    static const std::vector<IssuingAuthority> LICENSE_ISSUING_AUTHORITIES;
};
}
#endif // LicenseManagerKeys_h