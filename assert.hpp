#pragma once

#include <cstdlib>

#if defined(BOOST_COMP_MSVC)
#   define BK_DEBUG_BREAK __debugbreak
#else
#   error "define me" //TODO
#endif

#define BK_ASSERT(condition) \
do { \
    if (!(condition)) { \
        BK_DEBUG_BREAK(); \
        std::abort(); \
    } \
} while (!(condition))
