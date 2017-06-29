/**
 * @file
 * Declares a utility for retrieving the library version.
 */
#pragma once

#include <string>
#include "export.h"

namespace whereami {

    /**
     * Query the library version.
     * @return A version string with \<major>.\<minor>.\<patch>
     */
    std::string LIBWHEREAMI_EXPORT version();

}  // namespace whereami
