#include <gtest/gtest.h>
#include "mouse.hpp"

TEST(Mouse, DefaultConstruct) {
    bklib::mouse mouse;

    for (size_t i = 0; i < bklib::mouse::HISTORY_SIZE; ++i) {
        auto abs = mouse.absolute(i);
        auto rel = mouse.relative(i);

        ASSERT_EQ(abs.time , rel.time);

        ASSERT_EQ(abs.x, 0);
        ASSERT_EQ(abs.x, rel.x);

        ASSERT_EQ(abs.y, 0);
        ASSERT_EQ(abs.y, rel.y);
    }

    for (size_t i = 0; i < bklib::mouse::BUTTON_COUNT; ++i) {
        ASSERT_EQ(mouse.button(i).state, bklib::mouse::button_state::up);
    }
}

TEST(Mouse, SetGet) {
    bklib::mouse mouse;

    auto const now = bklib::clock_t::now();
    mouse.push_absolute(10, 20, now);
    mouse.push_relative(30, 40, now);

    ASSERT_EQ(mouse.absolute().x, 10);
    ASSERT_EQ(mouse.absolute().y, 20);
    ASSERT_EQ(mouse.absolute().time, now);

    ASSERT_EQ(mouse.relative().x, 30);
    ASSERT_EQ(mouse.relative().y, 40);
    ASSERT_EQ(mouse.relative().time, now);
}

//TODO

#include "window.hpp"

TEST(Mouse, Window) {
    bklib::platform_window win(L"hi");

    while (win.is_running()) {
        win.do_events();
    }

    auto result = win.get_result();
    
    ASSERT_TRUE(result.get());
}
