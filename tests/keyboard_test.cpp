#include "pch.hpp"
#include <gtest/gtest.h>
#include "keyboard.hpp"

using bklib::keyboard;
using bklib::keycode;

//==============================================================================
TEST(Keyboard, DefaultConstruct) {
    keyboard kb;

    for (size_t i = 0; i < keyboard::KEY_COUNT; ++i) {
        auto const& state = kb[static_cast<keycode>(i)];

        ASSERT_FALSE(state.is_down);
        ASSERT_FALSE(!!state);
    }
}
//==============================================================================
TEST(Keyboard, KeyName) {
    auto const name = keyboard::key_name(keycode::SPACE);
    ASSERT_STREQ(name.data(), "KEY::SPACE");
}
//==============================================================================
TEST(Keyboard, KeyCode) {
    char const name[] {"KEY::SPACE"};

    auto const code1 = keyboard::key_code(bklib::utf8string_hash(name));
    auto const code2 = keyboard::key_code(bklib::utf8string(name));
    auto const code3 = keyboard::key_code(bklib::string_ref(name));

    ASSERT_EQ(keycode::SPACE, code1);
    ASSERT_EQ(code1, code2);
    ASSERT_EQ(code2, code3);
}
//==============================================================================
TEST(Keyboard, SetState) {
    keyboard kb;

    ASSERT_FALSE(kb.set_state(keycode::SPACE, true));
    ASSERT_TRUE(kb.set_state(keycode::SPACE, true));    

    ASSERT_TRUE(!!kb[keycode::SPACE]);

    ASSERT_FALSE(kb.set_state(keycode::SPACE, false));
    ASSERT_TRUE(kb.set_state(keycode::SPACE, false));
}
//==============================================================================
TEST(Keyboard, Clear) {
    keyboard kb;

    kb.set_state(keycode::SPACE, true);

    auto const on_key_up = [](keyboard& k, keycode key) {
        ASSERT_EQ(key, keycode::SPACE);
    };

    kb.clear(on_key_up);
    ASSERT_FALSE(!!kb[keycode::SPACE]);
}
//==============================================================================
