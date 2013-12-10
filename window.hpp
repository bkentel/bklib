#pragma once

#include <memory>

#include "types.hpp"
#include "callback.hpp"
#include "math.hpp"
#include "util.hpp"

#include "keyboard.hpp"
#include "mouse.hpp"

#include "impl/platform.hpp"

namespace bklib {
//==============================================================================
#if defined(BOOST_OS_WINDOWS)
    struct platform_window_handle {
        operator HWND() const { return value; }
        HWND value;
    };
#else
#   error "define me"
#endif
//==============================================================================
BK_DECLARE_EVENT(on_create, void());
BK_DECLARE_EVENT(on_paint,  void());
BK_DECLARE_EVENT(on_close,  void());
BK_DECLARE_EVENT(on_resize, void(unsigned w, unsigned h));
//==============================================================================
//! Abstraction of a native window.
//==============================================================================
class platform_window {
public:
    using window_result = std::future<bool>;

    class impl_t_;
    //--------------------------------------------------------------------------
    //
    //--------------------------------------------------------------------------
    enum class state {
        starting, running, finished_error, finished_ok
    };
    //--------------------------------------------------------------------------
    platform_window(platform_window const&) = delete;
    platform_window& operator=(platform_window const&) = delete;

    ~platform_window();
    explicit platform_window(platform_string title);
    //--------------------------------------------------------------------------
    bool is_running() const;

    window_result get_result();

    void do_events();

    platform_window_handle get_handle() const;
    //--------------------------------------------------------------------------
    void listen(on_create        callback);
    void listen(on_paint         callback);
    void listen(on_close         callback);
    void listen(on_resize        callback);

    void listen(on_mouse_enter   callback);
    void listen(on_mouse_exit    callback);
    void listen(on_mouse_move    callback);
    void listen(on_mouse_move_to callback);
    void listen(on_mouse_down    callback);
    void listen(on_mouse_up      callback);
    void listen(on_mouse_wheel_v callback);
    void listen(on_mouse_wheel_h callback);

    void listen(on_keydown       callback);
    void listen(on_keyup         callback);
    //--------------------------------------------------------------------------
private:
    std::unique_ptr<impl_t_> impl_;
};

} //namespace bklib
