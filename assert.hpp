#pragma once

#include <cstdlib>
#include "macros.hpp"

#define BK_BREAK_AND_EXIT BK_DEBUG_BREAK(); std::abort

#define BK_IF_NOT_THEN(condition, then)\
[](bool const cond) {\
    if (!cond) {\
        then;\
    }\
}(!!(condition))


#define BK_PRECOND(condition) BK_IF_NOT_THEN((condition), BK_BREAK_AND_EXIT());
#define BK_ASSERT(condition) BK_IF_NOT_THEN((condition), BK_BREAK_AND_EXIT());

//#define BK_ASSERT(condition) \
//do { \
//    if (!(condition)) { \
//        BK_DEBUG_BREAK(); \
//        std::abort(); \
//    } \
//} while (!(condition))
