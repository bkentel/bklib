#include "pch.hpp"
#include <gtest/gtest.h>
#include "json.hpp"

namespace json = bklib::json;

struct JsonTest : public  ::testing::Test {
    template <size_t N>
    void parse(char const (&data)[N]) {
        auto const result = reader.parse(data, data + N, root);
        ASSERT_TRUE(result);

        ASSERT_NO_THROW(
            json::require_object(root)
        );
    }

    Json::Reader reader;
    Json::Value  root;
};

#define ASSERT_THROW_AND(expr, type, then)\
for (bool ASSERT_THROW_AND_passed_ = false; !ASSERT_THROW_AND_passed_; ) {\
    try {\
        expr;\
    } catch (type const& e) {\
        ASSERT_THROW_AND_passed_ = true;\
        then;\
    } catch (...) {\
        ASSERT_THROW(throw, type);\
    }\
} []{}()

//==============================================================================
// Test a valid key.
//==============================================================================
TEST_F(JsonTest, RequireKeyPass) {
    char const data[] {R"({
        "test": "test"
    })"};

    parse(data);

    ASSERT_NO_THROW(
        json::require_key(root, "test")
    );
}
//==============================================================================
// Test an invalid key.
//==============================================================================
TEST_F(JsonTest, RequireKeyFail) {
    char const data[] {R"({
        "test": "test"
    })"};

    char const index[] {"testt"};

    parse(data);

    ASSERT_THROW_AND(
        json::require_key(root, index)
      , json::error::bad_index, {
            auto const ptr = boost::get_error_info<json::error::info_index>(e);
            ASSERT_NE(ptr, nullptr);
            ASSERT_EQ(boost::get<bklib::utf8string>(*ptr), index);
        }
    );
}
//==============================================================================
// Test for an (valid) array.
//==============================================================================
TEST_F(JsonTest, RequireArrayPass) {
    char const data[] {R"({
        "test": ["test"]
    })"};

    parse(data);

    auto test = json::require_key(root, "test");

    ASSERT_NO_THROW(
        json::require_array(test)
    );
}
//==============================================================================
// Test for an (invalid) array.
//==============================================================================
TEST_F(JsonTest, RequireArrayFail) {
    char const data[] {R"({
        "test": "test"
    })"};

    parse(data);

    auto test = json::require_key(root, "test");

    ASSERT_THROW(
        json::require_array(test), json::error::bad_type
    );

    ASSERT_THROW_AND(
        json::require_array(test)
      , json::error::bad_type, {
            auto const ptr_expected = boost::get_error_info<json::error::info_expected_type>(e);
            auto const ptr_actual = boost::get_error_info<json::error::info_actual_type>(e);

            ASSERT_NE(ptr_expected, nullptr);
            ASSERT_NE(ptr_actual, nullptr);

            ASSERT_EQ(ptr_expected->type, Json::arrayValue);
            ASSERT_EQ(ptr_actual->type, Json::stringValue);
        }
    );
}
