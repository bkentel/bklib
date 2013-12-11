#pragma once

#include <cstdlib>
#include "macros.hpp"

#define BK_ASSERT(condition) \
do { \
    if (!(condition)) { \
        BK_DEBUG_BREAK(); \
        std::abort(); \
    } \
} while (!(condition))
