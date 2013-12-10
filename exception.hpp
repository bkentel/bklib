#pragma once

#include <exception>
#include <boost/exception/all.hpp>

namespace bklib {

struct exception_base : virtual boost::exception, virtual std::exception {};
struct platform_error : virtual exception_base {};
struct library_error  : virtual exception_base {};

} //namespace bklib

#define BK_DEFINE_EXCEPTION_INFO(name, type)\
namespace detail {\
    struct tag_##name;\
}\
using name = ::boost::error_info<detail::tag_##name, type>
