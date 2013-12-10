#pragma once

#include <string>
#include <functional>
#include <cstdint>
#include <chrono>

#include <boost/utility/string_ref.hpp>

#include "config.hpp"

namespace bklib {
    using int8_t   = ::std::int8_t;
    using int16_t  = ::std::int16_t;
    using int32_t  = ::std::int32_t;
    using int64_t  = ::std::int64_t;
    using uint8_t  = ::std::uint8_t;
    using uint16_t = ::std::uint16_t;
    using uint32_t = ::std::uint32_t;
    using uint64_t = ::std::uint64_t;

    using wstring_ref   = boost::wstring_ref;
    using string_ref    = boost::string_ref;
    using utf8string    = std::string;

    using hash_t = size_t;

    using clock_t    = std::chrono::high_resolution_clock;
    using time_point = clock_t::time_point;
#if defined(BOOST_OS_WINDOWS)
    using platform_char   = wchar_t;
    using platform_string = std::basic_string<platform_char>;
#else
    using platform_char   = char;
    using platform_string = std::basic_string<char>;
#endif
} //namespace bklib
