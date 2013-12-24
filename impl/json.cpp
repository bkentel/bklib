#include "json.hpp"
#include "util.hpp"
#include "assert.hpp"

namespace json   = bklib::json;
namespace error  = bklib::json::error;
namespace detail = bklib::json::detail;

using bklib::utf8string;
using bklib::string_ref;

using json::cref;
using json::cref_wrapped;

namespace {
    error::bad_type make_bad_type(
        Json::ValueType const expected
      , Json::ValueType const actual
    ) {
        error::bad_type e;

        e << error::info_expected_type{{expected}}
          << error::info_actual_type{{actual}};

        return e;
    }
} //namespace anon
//==============================================================================
void error::add_rule_exception_info(json::error::base& e, bklib::string_ref rule) {
    auto const ptr = boost::get_error_info<info_rule_trace>(e);

    if (ptr) {
        ptr->push_back(rule);
    } else {
        e << info_rule_trace{{rule}};
    }
}

//==============================================================================
string_ref json::type_info::to_string() const {
    using types = Json::ValueType;

    return [this] {
        switch (type) {
        default:
            //fall through to null
        case types::nullValue:
            return "null";
        case types::intValue:
            return "signed";
        case types::uintValue:
            return "unsigned";
        case types::realValue:
            return "float";
        case types::stringValue:
            return "string";
        case types::booleanValue:
            return "bool";
        case types::arrayValue:
            return "array";
        case types::objectValue:
            return "object";
        }
    }();
}
//==============================================================================
cref_wrapped json::require_array(cref json) {
    if (json.isArray()) {
        return json;
    }

    BOOST_THROW_EXCEPTION(
        make_bad_type(Json::arrayValue, json.type())
    );
}
//==============================================================================
cref_wrapped json::require_object(cref json) {
    if (json.isObject()) {
        return json;
    }

    BOOST_THROW_EXCEPTION(
        make_bad_type(Json::objectValue, json.type())
    );
}
//==============================================================================
cref_wrapped json::require_key(cref json, size_t index) {
    BK_ASSERT(json.isArray());

    if (json.size() >= index) {
        return json[index];
    }

    BOOST_THROW_EXCEPTION(
        error::bad_index{} << error::info_index{index}
    );   
}
//==============================================================================
cref_wrapped json::require_key(cref json, utf8string const& index) {
    BK_ASSERT(json.isObject());

    if (json.isMember(index)) {
        return json[index];
    }

    BOOST_THROW_EXCEPTION(
        error::bad_index{} << error::info_index{index}
    );
}
//==============================================================================
utf8string json::require_string(cref json) {
    if (json.isString()) {
        return json.asString();
    }

    BOOST_THROW_EXCEPTION(
        make_bad_type(Json::stringValue, json.type())
    );
}
//==============================================================================
int json::require_int(cref json) {
    if (json.isIntegral()) {
        return json.asInt();
    }

    BOOST_THROW_EXCEPTION(
        make_bad_type(Json::intValue, json.type())
    );
}

//==============================================================================
cref_wrapped json::optional_key(cref json, size_t const index) {
    BK_ASSERT(json.isArray());
    return (index < json.size()) ? json[index] : Json::Value::null;
}

//==============================================================================
std::ostream& error::operator<<(std::ostream& out, base const& e) {
    out << "json exception (" << typeid(e).name() << ")";

    if (auto const ptr = boost::get_error_info<info_expected_type>(e)) {
        out << "\n  expected type = " << ptr->to_string();
    }
    if (auto const ptr = boost::get_error_info<info_actual_type>(e)) {
        out << "\n  actual type   = " <<  ptr->to_string();
    }
    if (auto const ptr = boost::get_error_info<info_expected_size>(e)) {
        out << "\n  expected size = " << *ptr;
    }
    if (auto const ptr = boost::get_error_info<info_actual_size>(e)) {
        out << "\n  actual size   = " << *ptr;
    }
    if (auto const ptr = boost::get_error_info<info_index>(e)) {
        out << "\n  index         = " << *ptr;
    }

    out << std::endl;
    return out;
}
//==============================================================================
void json::detail::for_each_element_skip_on_fail_on_fail_(
    error::base const& e
  , size_t      const  index
) {
    BOOST_LOG_TRIVIAL(warning)
     << "parsing failed:"
     << "  element = " << index
     << "  reason  = " << e
    ;
}
//==============================================================================
