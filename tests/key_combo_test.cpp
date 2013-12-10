#include "pch.hpp"
#include <gtest/gtest.h>
#include "keyboard.hpp"

using bklib::key_combo;
using bklib::keycode;

//==============================================================================
TEST(KeyCombo, DefaultConstruct) {
    key_combo combo;

    ASSERT_TRUE(combo.empty());
    ASSERT_EQ(combo.size(), 0);
    ASSERT_FALSE(combo.remove(keycode::A));
    ASSERT_EQ(std::begin(combo), std::end(combo));
    ASSERT_EQ(std::cbegin(combo), std::cend(combo));
    ASSERT_EQ(combo, combo);
}
//==============================================================================
TEST(KeyCombo, ListConstruct) {
    key_combo const combo {keycode::CTRL_L, keycode::C};

    ASSERT_FALSE(combo.empty());
    ASSERT_EQ(combo.size(), 2);
    ASSERT_EQ(combo, combo);

    ASSERT_NE(
        std::find(std::cbegin(combo), std::cend(combo), keycode::CTRL_L)
      , std::cend(combo)
    );

    ASSERT_NE(
        std::find(std::cbegin(combo), std::cend(combo), keycode::C)
      , std::cend(combo)
    );
}
//==============================================================================
TEST(KeyCombo, Add) {
    key_combo combo;

    ASSERT_TRUE(combo.add(keycode::CTRL_L));
    ASSERT_EQ(combo.size(), 1);

    ASSERT_TRUE(combo.add({keycode::C, keycode::ALT_L}));
    ASSERT_EQ(combo.size(), 3);

    ASSERT_NE(
        std::find(std::cbegin(combo), std::cend(combo), keycode::CTRL_L)
      , std::cend(combo)
    );

    ASSERT_NE(
        std::find(std::cbegin(combo), std::cend(combo), keycode::ALT_L)
      , std::cend(combo)
    );

    ASSERT_NE(
        std::find(std::cbegin(combo), std::cend(combo), keycode::C)
      , std::cend(combo)
    );
}
//==============================================================================
TEST(KeyCombo, Remove) {
    key_combo combo {keycode::CTRL_L, keycode::ALT_L, keycode::C};

    combo.remove(keycode::CTRL_L);
    ASSERT_EQ(combo.size(), 2);
    ASSERT_EQ(
        std::find(std::cbegin(combo), std::cend(combo), keycode::CTRL_L)
      , std::cend(combo)
    );

    combo.remove(keycode::ALT_L);
    ASSERT_EQ(combo.size(), 1);
    ASSERT_EQ(
        std::find(std::cbegin(combo), std::cend(combo), keycode::ALT_L)
      , std::cend(combo)
    );

    combo.remove(keycode::C);
    ASSERT_EQ(combo.size(), 0);
    ASSERT_EQ(
        std::find(std::cbegin(combo), std::cend(combo), keycode::C)
      , std::cend(combo)
    );
}
//==============================================================================
TEST(KeyCombo, Clear) {
    key_combo combo {keycode::CTRL_L, keycode::ALT_L, keycode::C};
    combo.clear();

    ASSERT_EQ(combo.size(), 0);
    ASSERT_TRUE(combo.empty());
}
//==============================================================================
TEST(KeyCombo, Equality) {
    key_combo const combo1 {keycode::CTRL_L, keycode::ALT_L, keycode::C};
    key_combo const combo2 {keycode::CTRL_L, keycode::ALT_L, keycode::D};
    key_combo const combo3 {keycode::CTRL_L, keycode::ALT_L, keycode::C};

    ASSERT_EQ(combo1, combo1);
    ASSERT_EQ(combo2, combo2);
    ASSERT_EQ(combo1, combo3);
    ASSERT_EQ(combo3, combo1);

    ASSERT_NE(combo1, combo2);
    ASSERT_NE(combo2, combo1);

    ASSERT_GE(combo1, combo1);
    ASSERT_LT(combo1, combo2);
}
