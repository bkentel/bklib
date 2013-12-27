#pragma once

#include <boost/variant.hpp>
#include "types.hpp"

namespace Json {
    class Value;
} //namespace Json

namespace bklib {
    namespace json {
        using cref_wrapped = std::reference_wrapper<Json::Value const>;
        using cref         = Json::Value const&;
        using index        = boost::variant<size_t, utf8string>;
        using type         = Json::ValueType;

        struct type_info;
    } //namespace json
} //namespace bklib
