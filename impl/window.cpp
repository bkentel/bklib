#include "window.hpp"

#if defined (BOOST_OS_WINDOWS)
#   include "impl/win/win_window.hpp"
#else
#   error "fill me in"
#endif

using bklib::platform_window;
using bklib::mouse;
using bklib::keyboard;
using bklib::platform_window_handle;

////////////////////////////////////////////////////////////////////////////////
// bklib::platform_window
////////////////////////////////////////////////////////////////////////////////
platform_window::~platform_window() {
}

platform_window::platform_window(
    bklib::platform_string title
)
  : impl_ {std::make_unique<impl_t_>()}
{
}

bool platform_window::is_running() const {
    return impl_->is_running();
}

platform_window::window_result platform_window::get_result() {
    return impl_->get_result();
}

void platform_window::do_events() {
    impl_->do_events();
}

platform_window_handle platform_window::get_handle() const {
    return impl_->get_handle();
}

#define BK_DEFINE_EVENT(EVENT)\
void platform_window::listen(EVENT callback) {\
    impl_->listen(callback);\
}

BK_DEFINE_EVENT(bklib::on_create)
BK_DEFINE_EVENT(bklib::on_paint)
BK_DEFINE_EVENT(bklib::on_close)
BK_DEFINE_EVENT(bklib::on_resize)
BK_DEFINE_EVENT(bklib::on_mouse_enter)
BK_DEFINE_EVENT(bklib::on_mouse_exit)
BK_DEFINE_EVENT(bklib::on_mouse_move)
BK_DEFINE_EVENT(bklib::on_mouse_move_to)
BK_DEFINE_EVENT(bklib::on_mouse_down)
BK_DEFINE_EVENT(bklib::on_mouse_up)
BK_DEFINE_EVENT(bklib::on_mouse_wheel_v)
BK_DEFINE_EVENT(bklib::on_mouse_wheel_h)
BK_DEFINE_EVENT(bklib::on_keydown)
BK_DEFINE_EVENT(bklib::on_keyup)
//BK_DEFINE_EVENT(bklib::ime_candidate_list::on_begin)
//BK_DEFINE_EVENT(bklib::ime_candidate_list::on_update)
//BK_DEFINE_EVENT(bklib::ime_candidate_list::on_end)

#undef BK_DEFINE_EVENT
