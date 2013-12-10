#pragma once

#include <boost/predef.h>

#if defined(BOOST_COMP_MSVC)
#   if BOOST_COMP_MSVC < BOOST_VERSION_NUMBER(12,0,21005)
#       define BK_NOEXCEPT throw()
#       define BK_NOEXCEPT_OP(x)
#       define BK_CONSTEXPR
#   else if BOOST_COMP_MSVC < BOOST_VERSION_NUMBER(13,0,0)
#       define BK_NOEXCEPT noexcept
#       define BK_NOEXCEPT_OP(x)
#       define BK_CONSTEXPR constexpr
#   endif
#endif
