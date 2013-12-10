#pragma once

#include "win_platform.hpp"
#include "concurrent_queue.hpp"
#include "window.hpp"
#include "exception.hpp"
#include "macros.hpp"

namespace bklib {

namespace detail {
//==============================================================================
//!
//==============================================================================
class raw_mouse {
public:
    static BK_CONSTEXPR size_t const BUTTON_COUNT = 5;

    enum class button_state {
        no_change = 0, went_down = 1, went_up = 2
    };

    explicit raw_mouse(RAWMOUSE const& mouse)
      : data_{mouse}
    {
    }

    button_state operator[](size_t button) const {
        BK_ASSERT(button < 6);

        auto const flags = data_.usButtonFlags;
        auto const mask  = 3 << 2*button;
        auto const shift = 2*button;

        auto const result = (flags & mask) >> shift;
        BK_ASSERT(result < 3);

        return static_cast<button_state>(result);
    }

    bool has_movement() const BK_NOEXCEPT {
        return data_.lLastX || data_.lLastY;
    }

    bool has_buttons() const BK_NOEXCEPT {
        return (data_.ulButtons & (RI_MOUSE_WHEEL - 1)) != 0;
    }

    bool has_wheel() const BK_NOEXCEPT {
        return (data_.ulButtons & RI_MOUSE_WHEEL) != 0;
    }

    int16_t wheel_delta() const BK_NOEXCEPT {
        return static_cast<SHORT>(data_.usButtonData);  
    }

    int16_t x() const BK_NOEXCEPT {
        return static_cast<int16_t>(data_.lLastX);
    }

    int16_t y() const BK_NOEXCEPT {
        return static_cast<int16_t>(data_.lLastY);
    }
private:
    RAWMOUSE data_;
};
//==============================================================================
//!
//==============================================================================
class raw_keyboard {
public:
    explicit raw_keyboard(RAWKEYBOARD const& keyboard);

    bool    went_down() const { BK_ASSERT(is_valid()); return went_down_; }
    bool    is_valid()  const { return is_valid_; }
    keycode key()       const { BK_ASSERT(is_valid()); return key_; }
    USHORT  scancode()  const { BK_ASSERT(is_valid()); return scancode_; }

    static wstring_ref get_key_name(UINT scancode);
private:
    RAWKEYBOARD data_;
    
    bool    went_down_ = false;
    bool    is_valid_  = false;
    keycode key_       = keycode::NONE;
    USHORT  scancode_  = 0;
};
//==============================================================================
//! Helper class for raw_input messages.
//! @attention not thread safe
//==============================================================================
class raw_input {
public:
    //--------------------------------------------------------------------------
    explicit raw_input(LPARAM lParam);
    //--------------------------------------------------------------------------
    bool is_mouse()    const BK_NOEXCEPT { return get_().header.dwType == RIM_TYPEMOUSE; }
    bool is_keyboard() const BK_NOEXCEPT { return get_().header.dwType == RIM_TYPEKEYBOARD; }
    bool is_hid()      const BK_NOEXCEPT { return get_().header.dwType == RIM_TYPEHID; }
    
    RAWKEYBOARD const& keyboard() const BK_NOEXCEPT {
        BK_ASSERT(is_keyboard());
        return get_().data.keyboard;
    }

    RAWMOUSE const& mouse() const BK_NOEXCEPT {
        BK_ASSERT(is_mouse());
        return get_().data.mouse;
    }

    RAWHID const& hid() const BK_NOEXCEPT {
        BK_ASSERT(is_hid());
        return get_().data.hid;
    }

    void handle_message();
private:
    RAWINPUT const& get_() const BK_NOEXCEPT {
        return *reinterpret_cast<RAWINPUT*>(buffer_.data());
    }

    //! Reused buffer. Not thread safe.
    static std::vector<char> buffer_;
};
//==============================================================================
} //namespace detail

//==============================================================================
//! Windows implementation for bklib::platform_window.
//==============================================================================
class platform_window::impl_t_ {
    BK_NO_COPY_ASSIGN(impl_t_);
public:
    using work_item  = std::function<void()>;
    using work_queue = concurrent_queue<work_item>;

    impl_t_();

    void shutdown();

    HWND handle() const BK_NOEXCEPT { return window_.get(); }

    bool is_running() const BK_NOEXCEPT { return state_ == state::running; }

    window_result get_result() {
        return windows_result_.get_future();
    }

    void do_events();

    platform_window_handle get_handle() const;

    //--------------------------------------------------------------------------
    void listen(on_create        callback);
    void listen(on_paint         callback);
    void listen(on_close         callback);
    void listen(on_resize        callback);
    //--------------------------------------------------------------------------
    void listen(on_mouse_enter   callback);
    void listen(on_mouse_exit    callback);
    void listen(on_mouse_move    callback);
    void listen(on_mouse_move_to callback);
    void listen(on_mouse_down    callback);
    void listen(on_mouse_up      callback);
    void listen(on_mouse_wheel_v callback);
    void listen(on_mouse_wheel_h callback);
    //--------------------------------------------------------------------------
    void listen(on_keydown       callback);
    void listen(on_keyup         callback);
    //--------------------------------------------------------------------------
private:
    detail::window_handle window_;

    mouse    mouse_state_;
    keyboard keyboard_state_;
    //--------------------------------------------------------------------------
    on_create        on_create_;
    on_paint         on_paint_;
    on_close         on_close_;
    on_resize        on_resize_;
    //--------------------------------------------------------------------------
    on_mouse_move_to on_mouse_move_to_;
    on_mouse_move    on_mouse_move_;
    on_mouse_down    on_mouse_down_;
    on_mouse_up      on_mouse_up_;
    on_mouse_wheel_v on_mouse_wheel_v_;
    on_mouse_wheel_h on_mouse_wheel_h_;
    //--------------------------------------------------------------------------
    on_keydown       on_keydown_;
    on_keyup         on_keyup_;
    on_keyrepeat     on_keyrepeat_;
    //--------------------------------------------------------------------------

    LRESULT local_wnd_proc_(UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT handle_wm_input_(WPARAM wParam, LPARAM lParam);

    void handle_raw_mouse_(detail::raw_mouse const& mouse, time_point when);
    void handle_raw_keyboard_(detail::raw_keyboard const& keyboard, time_point when);
private:
    static LRESULT CALLBACK wnd_proc_(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) BK_NOEXCEPT;
    static HWND create_window_(impl_t_* win);

    static void main_();
    static void init_();

    //! add a job to be executed by the windows thread.
    static void push_work_item_(work_item item);
    //! add a job (event) to be executed by the client thread.
    static void push_event_item_(work_item item);

    static work_queue         windows_queue_;
    static work_queue         client_queue_;
    static DWORD              thread_id_;
    static state              state_;
    static std::promise<bool> windows_result_;
};
//==============================================================================

} //namespace bklib
