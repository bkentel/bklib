#pragma once

#include "config.hpp"

#if defined(BOOST_OS_WINDOWS)
#   include "impl/win/win_platform.hpp"
#else
#   error "fill me in"
#endif
