#pragma once

#include <type_traits>
#include <ostream>

#include <jsoncpp/json.h>

#include "types.hpp"
#include "exception.hpp"
#include "json_forward.hpp"

namespace bklib {
namespace json {
//==============================================================================
struct type_info {
    bool operator==(type_info const& rhs) BK_NOEXCEPT { return type == rhs.type; }
    bool operator!=(type_info const& rhs) BK_NOEXCEPT { return type != rhs.type; }

    string_ref to_string() const;

    Json::ValueType type;
};
//==============================================================================
namespace error {
    struct base      : virtual bklib::exception_base {};
    struct bad_type  : virtual base {};
    struct bad_size  : virtual base {};
    struct bad_index : virtual base {};

    BK_DEFINE_EXCEPTION_INFO(info_expected_type, type_info);
    BK_DEFINE_EXCEPTION_INFO(info_actual_type,   type_info);
    BK_DEFINE_EXCEPTION_INFO(info_expected_size, size_t);
    BK_DEFINE_EXCEPTION_INFO(info_actual_size,   size_t);
    BK_DEFINE_EXCEPTION_INFO(info_index,         index);
    BK_DEFINE_EXCEPTION_INFO(info_rule_trace,    std::vector<bklib::string_ref>);

    void add_rule_exception_info(json::error::base& e, bklib::string_ref rule);

    std::ostream& operator<<(std::ostream& out, base const& e);
} //namespace error

#define BK_JSON_ADD_TRACE(E) ::bklib::json::error::add_rule_exception_info(E, __func__); throw

//==============================================================================
//! @throws json::error::bad_type if !json.isArray().
//! @return json
//==============================================================================
cref_wrapped require_array(cref json);
//==============================================================================
//! @throws json::error::bad_type if !json.isObject().
//! @return json
//==============================================================================
cref_wrapped require_object(cref json);
//==============================================================================
//! @pre json.isObject().
//! @throws json::error::bad_index if @c index is invalid.
//! @return A @c cref to the value at @c index.
//==============================================================================
cref_wrapped require_key(cref json, utf8string const& index);
//==============================================================================
//! @pre json.isArray().
//! @throws json::error::bad_index if @c index is invalid.
//! @return A @c cref to the value at @c index.
//==============================================================================
cref_wrapped require_key(cref json, size_t index);
//==============================================================================
//! @throws json::error::bad_type if @c !json.isString().
//! @return @c json as a string.
//==============================================================================
utf8string require_string(cref json);
//==============================================================================

int require_int(cref json);

cref_wrapped optional_key(cref json, size_t index);


namespace detail {
    void for_each_element_skip_on_fail_on_fail_(
        error::base const& e
      , size_t      const  index
    );
} //namespace detail

//==============================================================================
//! Iterate through each array element in @c json and apply the unary function
//! @action to each element. If @c action fails due to a json related error, the
//! error is logged and iteration continues at the next element.
//!
//! @throws json::error::bad_type if !json.isArray().
//==============================================================================
template <typename F>
void for_each_element_skip_on_fail(cref json, F&& action) {
    json::require_array(json);

    auto const size = json.size();
    for (size_t i = 0; i < size; ++i) {
        try {
            action(require_key(json, i));
        } catch (error::base const& e) {
            detail::for_each_element_skip_on_fail_on_fail_(e, i);
        }
    }
}
//==============================================================================

} //namespace json
} //namespace bklib
