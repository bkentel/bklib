#include "win_window.hpp"
#include "util.hpp"
#include "scope_exit.hpp"

#define BK_WM_ASSOCIATE_TSF (WM_APP + 1)

using window_impl = bklib::platform_window::impl_t_;
using bklib::concurrent_queue;
using bklib::platform_window;
using bklib::platform_window_handle;
using bklib::detail::raw_input;
using bklib::detail::raw_mouse;
using bklib::detail::raw_keyboard;

using bklib::detail::make_windows_error;

//------------------------------------------------------------------------------
// window_impl statics
//------------------------------------------------------------------------------
window_impl::work_queue window_impl::windows_queue_  { };
window_impl::work_queue window_impl::client_queue_   { };
DWORD                   window_impl::thread_id_      {0};
platform_window::state  window_impl::state_          {platform_window::state::starting};
std::promise<bool>      window_impl::windows_result_ { };
//------------------------------------------------------------------------------
// raw_input statics
//------------------------------------------------------------------------------
std::vector<char> raw_input::buffer_ { };
//------------------------------------------------------------------------------
namespace {
    //--------------------------------------------------------------------------
    //! Keyboard key names. 
    std::vector<std::wstring> key_names { };
    //--------------------------------------------------------------------------
    std::once_flag windows_once_flag;   //used for initialization
    //--------------------------------------------------------------------------
    void init_key_names() {
        BK_ASSERT(key_names.empty());

        static size_t const BUF_SIZE  = 64;
        static UINT   const KEY_COUNT = 0x200;

        wchar_t name_buffer[BUF_SIZE];

        key_names.reserve(KEY_COUNT);

        for (UINT scancode = 0; scancode < KEY_COUNT; ++scancode) {
            auto const length = ::GetKeyNameTextW(
                (scancode << 16), name_buffer, BUF_SIZE
            );

            if (length == 0) {
                //BK_DEBUG_BREAK(); //TODO
                std::cout << scancode << std::endl;
            }

            key_names.emplace_back(name_buffer, name_buffer + length);
        }
    }
    //--------------------------------------------------------------------------
    template <typename T>
    T* get_user_data(HWND hWnd) {
        ::SetLastError(0);
        auto const result = ::GetWindowLongPtrW(hWnd, GWLP_USERDATA);

        if (result == 0) {
            auto const e = ::GetLastError();
            if (e != 0) {
                BOOST_THROW_EXCEPTION(
                    make_windows_error("GetWindowLongPtrW")
                );
            }
        }

        return reinterpret_cast<T*>(result);
    }
    //--------------------------------------------------------------------------
    template <typename T>
    LONG set_user_data(HWND hWnd, T* data) {
        ::SetLastError(0);
        auto const value = reinterpret_cast<LONG>(data);
        auto const result = ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, value);

        if (result == 0) {
            auto const e = ::GetLastError();
            if (e != 0) {
                BOOST_THROW_EXCEPTION(
                    make_windows_error("GetWindowLongPtrW")
                );
            }
        }

        return result;
    }
    //--------------------------------------------------------------------------
    void init_com() {
        BK_CHECK_COM(
            ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)
          , "CoInitializeEx"
        );

        auto options = bklib::detail::make_com_ptr([]{
            IGlobalOptions* result = nullptr;

            BK_CHECK_COM(
                ::CoCreateInstance(
                    CLSID_GlobalOptions
                  , nullptr
                  , CLSCTX_INPROC_SERVER
                  , IID_PPV_ARGS(&result)
                )
              , "CoCreateInstance"
            );

            return result;
        });

        BK_CHECK_COM(
            options->Set(COMGLB_EXCEPTION_HANDLING, COMGLB_EXCEPTION_DONOT_HANDLE_ANY)
          , "IGlobalOptions::Set"
        );
    }
    //--------------------------------------------------------------------------
    void init_raw_input() {
        static RAWINPUTDEVICE const devices[] = {
            {0x01, 0x02, 0, 0} //mouse
          , {0x01, 0x06, 0, 0} //keyboard
        };

        auto const result = ::RegisterRawInputDevices(
            devices
          , bklib::elements_in(devices)
          , sizeof(RAWINPUTDEVICE)
        );

        if (result == FALSE) {
            BOOST_THROW_EXCEPTION(
                make_windows_error("RegisterRawInputDevices")
            );
        }

        init_key_names();
    }
    //--------------------------------------------------------------------------
    void init_heap() {
        ::HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);
    }
    //--------------------------------------------------------------------------
    void init_ime() {
        auto const result = ::ImmDisableIME(static_cast<DWORD>(-1));
        if (result == FALSE) {
            BOOST_THROW_EXCEPTION(
                make_windows_error("ImmDisableIME")
            );
        }
    }
    //--------------------------------------------------------------------------
} //namespace
//------------------------------------------------------------------------------
void window_impl::push_work_item_(work_item item) {
    windows_queue_.push(std::move(item));

    auto const result = ::PostThreadMessageW(thread_id_, WM_NULL, 0, 0);
    if (result == 0) {
        BOOST_THROW_EXCEPTION(
            make_windows_error("PostThreadMessageW")
        );
    }
}
//------------------------------------------------------------------------------
void window_impl::push_event_item_(work_item item) {
    client_queue_.push(std::move(item));
}
//------------------------------------------------------------------------------
void window_impl::init_() {
    std::call_once(windows_once_flag, [] {
        init_heap();
        init_com();
        init_raw_input();
        init_ime();

        std::thread window_thread(window_impl::main_);
        thread_id_ = ::GetThreadId(window_thread.native_handle());
        window_thread.detach();

        while (state_ == state::starting) {
            ::Sleep(1);
        }
    });
}
//------------------------------------------------------------------------------
void window_impl::main_()
try {
    BK_ASSERT(state_ == state::starting);
    BK_SCOPE_EXIT_NAME( on_scope_exit, {
        state_ = state::finished_error;
        windows_result_.set_value_at_thread_exit(false);
    });

    init_com();

    MSG msg {0};

    //enusure the thread has a message queue before continuing
    ::PeekMessageW(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    //ime_manager_ = std::make_unique<impl::ime_manager>(); //TODO

    state_ = state::running;
    while (state::running == state_) {
        auto const result = ::GetMessageW(&msg, 0, 0, 0);

        while (!windows_queue_.is_empty()) {
            windows_queue_.pop()();
        }

        if (result == FALSE) {
            break;  //WM_QUIT
        } else if (result == -1) {
            return; //error
        }

        if (msg.message == BK_WM_ASSOCIATE_TSF) {
            //ime_manager_->associate(msg.hwnd); //TODO
        }

        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }

    on_scope_exit.cancel();

    state_ = state::finished_ok;
    windows_result_.set_value_at_thread_exit(true);
} catch (...) {
    windows_result_.set_exception_at_thread_exit(std::current_exception());
}
//------------------------------------------------------------------------------
HWND window_impl::create_window_(window_impl* win) {
    static wchar_t const CLASS_NAME[] = L"bkwin";

    init_();

    std::promise<HWND> promise_window;
    std::future<HWND>  future_window = promise_window.get_future();

    push_work_item_([&] {
        auto const instance = ::GetModuleHandleW(nullptr);

        WNDCLASSEXW const wc = {
            sizeof(WNDCLASSEXW),
            CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
            window_impl::wnd_proc_,
            0,
            0,
            instance,
            ::LoadIconW(nullptr, MAKEINTRESOURCEW(IDI_WINLOGO)),
            ::LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW)),
            nullptr,
            nullptr,
            CLASS_NAME,
            nullptr
        };

        ::RegisterClassExW(&wc); // ignore return value

        auto const result = ::CreateWindowExW(
            0,
            CLASS_NAME,
            L"window",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            (HWND)nullptr,
            (HMENU)nullptr,
            instance,
            win
        );

        if (result == nullptr) {
            BOOST_THROW_EXCEPTION(
                make_windows_error("CreateWindowExW")
            );
        }

        //::PostMessageW(result, BK_WM_ASSOCIATE_TSF, reinterpret_cast<WPARAM>(result), 0);

        promise_window.set_value(result);
    });

    return future_window.get();
}
//------------------------------------------------------------------------------
LRESULT CALLBACK window_impl::wnd_proc_(HWND hWnd, UINT const uMsg, WPARAM wParam, LPARAM lParam) BK_NOEXCEPT {
    try {
        if (uMsg == WM_NCCREATE) {
            auto const create_struct = reinterpret_cast<LPCREATESTRUCTW const>(lParam);
            auto win = reinterpret_cast<UNALIGNED window_impl*>(create_struct->lpCreateParams);

            auto const old = set_user_data(hWnd, win);
            BK_ASSERT(old == 0);

            BK_ASSERT(win->window_ == nullptr);
            win->window_.reset(hWnd);
        }

        auto const win = get_user_data<window_impl>(hWnd);
        if (win == nullptr) {
            return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
        }

        return win->local_wnd_proc_(uMsg, wParam, lParam);
    } catch (std::exception& e) {
        BK_UNUSED(e);
        BK_DEBUG_BREAK();
        ::PostQuitMessage(-1);
    } catch (...) {
        BK_DEBUG_BREAK();
        ::PostQuitMessage(-1);
    }

    return 0;
}
//------------------------------------------------------------------------------
void window_impl::handle_raw_mouse_(
    raw_mouse const&        mouse
  , bklib::time_point const when
) {
    if (mouse.has_movement()) {
        mouse_state_.push_relative(mouse.x(), mouse.y(), when);
        if (on_mouse_move_) {
            on_mouse_move_(mouse_state_, mouse.x(), mouse.y());
        }
    }

    if (mouse.has_buttons()) {
        auto const ms = mouse_state_.absolute();
        auto const x = ms.x;
        auto const y = ms.y;

        for (size_t i = 0; i < raw_mouse::BUTTON_COUNT; ++i) {
            switch (mouse[i]) {
            case raw_mouse::button_state::went_down :
                mouse_state_.set_button(i, when, true);
                if (on_mouse_down_) on_mouse_down_(mouse_state_, x, y, i);
                break;
            case raw_mouse::button_state::went_up :
                mouse_state_.set_button(i, when, false);
                if (on_mouse_up_) on_mouse_up_(mouse_state_, x, y, i);
                break;
            }
        }
    }

    if (mouse.has_wheel() && on_mouse_wheel_v_) {
        on_mouse_wheel_v_(mouse_state_, mouse.wheel_delta());
    }
}
//------------------------------------------------------------------------------
void window_impl::handle_raw_keyboard_(
    raw_keyboard const&     keyboard
  , bklib::time_point const when
) {
    auto const went_down = keyboard.went_down();
    auto const key       = keyboard.key();

    auto const repeat = keyboard_state_.set_state(key, went_down);

    if (repeat) {
        if (on_keyrepeat_) on_keyrepeat_(keyboard_state_, key);
    } else if (went_down && on_keydown_) {
        on_keydown_(keyboard_state_, key);
    } else if (!went_down && on_keyup_) {
        on_keyup_(keyboard_state_, key);
    }
}
//------------------------------------------------------------------------------
LRESULT window_impl::handle_wm_input_(WPARAM wParam, LPARAM lParam) {
    raw_input  input {lParam};
    auto const time = clock_t::now();

    if (input.is_mouse()) {
        raw_mouse mouse {input.mouse()};

        push_event_item_([=] {
            handle_raw_mouse_(mouse, time);
        });        
    } else if (input.is_keyboard()) {
        raw_keyboard keyboard {input.keyboard()};

        if (keyboard.is_valid()) {
            push_event_item_([=] {
                handle_raw_keyboard_(keyboard, time);
            });                
        }
    } else if (input.is_hid()) {
        //TODO
    }

    input.handle_message();
    return 0;
}
//------------------------------------------------------------------------------
LRESULT window_impl::local_wnd_proc_(
    UINT   const uMsg
  , WPARAM const wParam
  , LPARAM const lParam
) {
    //--------------------------------------------------------------------------
    // WM_PAINT
    //--------------------------------------------------------------------------
    auto const handle_paint = [&]() -> LRESULT {
        push_event_item_([this] {
            if (on_paint_) on_paint_();
        });

        ::ValidateRect(handle(), nullptr);

        return 0;
    };
    //--------------------------------------------------------------------------
    // WM_MOUSEMOVE
    //--------------------------------------------------------------------------
    auto const handle_mouse_move = [&]() -> LRESULT {
        auto const x    = static_cast<int16_t>(lParam & 0xFFFF);
        auto const y    = static_cast<int16_t>((lParam >> 16) & 0xFFFF);
        auto const time = clock_t::now();

        push_event_item_([=] {
            mouse_state_.push_absolute(x, y, time);

            if (on_mouse_move_to_) {
                on_mouse_move_to_(mouse_state_, x, y);
            }
        });

        return 0;
    };
    //--------------------------------------------------------------------------
    // WM_SIZE
    //--------------------------------------------------------------------------
    auto const handle_size = [&]() -> LRESULT {
        push_event_item_([this] {
            RECT r {};
            ::GetClientRect(handle(), &r);
            
            if (on_resize_) on_resize_(
                r.right  - r.left
              , r.bottom - r.top
            );
        });

        return 0;
    };
    //--------------------------------------------------------------------------

    switch (uMsg) {
    default :
        std::cout << std::hex <<  uMsg << std::endl;
        break;
    case WM_SETFOCUS :
        break;
    case WM_KILLFOCUS :
        keyboard_state_.clear(on_keyup_);
        break;
    case WM_MOUSEHWHEEL:
    case WM_MOUSEWHEEL :
        break;
    case WM_INPUT : 
        return handle_wm_input_(wParam, lParam);
        break;
    case WM_PAINT :
        return handle_paint();
        break;
    case WM_ERASEBKGND :
        return 1;
        break;
    case WM_DESTROY :
        ::PostQuitMessage(0);
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
        break;
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
        break;
    case WM_MOUSEMOVE :
        return handle_mouse_move();
        break;
    case WM_SIZE:
        return handle_size();
        break;
    }

    return ::DefWindowProcW(window_.get(), uMsg, wParam, lParam);
}
//------------------------------------------------------------------------------
window_impl::impl_t_()
    : window_ {nullptr}
{
    auto const result = create_window_(this);
    BK_ASSERT(result == window_.get());

    ::UpdateWindow(handle());
    ::ShowWindow(handle(), SW_SHOWDEFAULT);
}
//------------------------------------------------------------------------------
void window_impl::do_events() {
    while (!client_queue_.is_empty()) {
        client_queue_.pop()();
    }
}
//------------------------------------------------------------------------------
bklib::platform_window_handle window_impl::get_handle() const {
    return {handle()};
}
//------------------------------------------------------------------------------
void window_impl::shutdown() {
    push_work_item_([] {
        ::PostQuitMessage(0);
    });
}
//------------------------------------------------------------------------------
#define BK_DEFINE_EVENT(event)\
void window_impl::listen(bklib::event callback) {\
    event##_ = callback;\
}

BK_DEFINE_EVENT(on_create);
BK_DEFINE_EVENT(on_paint);
BK_DEFINE_EVENT(on_close);
BK_DEFINE_EVENT(on_resize);
//------------------------------------------------------------------------------
void window_impl::listen(bklib::on_mouse_enter   callback) {}
void window_impl::listen(bklib::on_mouse_exit    callback) {}
BK_DEFINE_EVENT(on_mouse_move);
BK_DEFINE_EVENT(on_mouse_move_to);
BK_DEFINE_EVENT(on_mouse_down);
BK_DEFINE_EVENT(on_mouse_up);
BK_DEFINE_EVENT(on_mouse_wheel_v);
void window_impl::listen(bklib::on_mouse_wheel_h callback) {}
//------------------------------------------------------------------------------
BK_DEFINE_EVENT(on_keydown);
BK_DEFINE_EVENT(on_keyup);
BK_DEFINE_EVENT(on_keyrepeat);

#undef BK_DEFINE_EVENT
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// raw_input
////////////////////////////////////////////////////////////////////////////////
using raw_input = bklib::detail::raw_input;
//------------------------------------------------------------------------------
raw_input::raw_input(LPARAM const lParam)
{
    auto const handle = reinterpret_cast<HRAWINPUT>(lParam);

    size_t buffer_size {0};

    auto result = ::GetRawInputData(
        handle
      , RID_INPUT
      , 0
      , &buffer_size
      , sizeof(RAWINPUTHEADER)
    );

    if (result != 0) {
        BOOST_THROW_EXCEPTION(
            make_windows_error("GetRawInputData")
        );
    }

    if (buffer_.size() < buffer_size) {
        buffer_.resize(buffer_size);
    }

    result = ::GetRawInputData(
        handle
      , RID_INPUT
      , buffer_.data()
      , &buffer_size
      , sizeof(RAWINPUTHEADER)
    );

    if (result != buffer_size) {
        BOOST_THROW_EXCEPTION(
            make_windows_error("GetRawInputData")
        );
    }
}
//------------------------------------------------------------------------------
void raw_input::handle_message() {
    auto ptr = reinterpret_cast<RAWINPUT*>(buffer_.data());
    auto const result = ::DefRawInputProc(&ptr, 1, sizeof(RAWINPUTHEADER));
    if (result != S_OK) {
        BOOST_THROW_EXCEPTION(
            make_windows_error("DefRawInputProc")
        );
    }
}
//------------------------------------------------------------------------------
raw_keyboard::raw_keyboard(RAWKEYBOARD const& keyboard)
  : data_(keyboard)
{
    auto const& kb = data_;

    if (kb.VKey == 0xFF) {
        return;
    } else {
        is_valid_ = true; 
    }

    bool const went_down_ = !(kb.Flags & RI_KEY_BREAK);
    bool const is_e0      =  (kb.Flags & RI_KEY_E0) != 0;
    bool const is_e1      =  (kb.Flags & RI_KEY_E1) != 0;

    scancode_ = [&] {
        USHORT result = is_e0 ? (kb.MakeCode | 0x100) : (kb.MakeCode);

        // pause is a special case... bug in the API
        if (kb.VKey == VK_PAUSE) {
            result = static_cast<USHORT>(0x45);
        // numlock is another special case
        } else if (
            kb.VKey == VK_NUMLOCK || kb.VKey == VK_LWIN
         || kb.VKey == VK_RWIN    || kb.VKey == VK_APPS
        ) {
            result = static_cast<USHORT>(
                ::MapVirtualKeyW(kb.VKey, MAPVK_VK_TO_VSC) | 0x100
            );
        }
        //else if (kb.VKey == VK_SNAPSHOT) {
        //    return static_cast<USHORT>(
        //        ::MapVirtualKeyW(kb.VKey, MAPVK_VK_TO_VSC)
        //    );
        //}

        return result;
    }();
       
    // virtual key code
    auto const vkey = [&] {
        UINT const flag = is_e0 ? (0xE0 << 8) :
                          is_e1 ? (0xE1 << 8) : 0;

        auto const key = ::MapVirtualKeyExW(
            scancode_ | flag, MAPVK_VSC_TO_VK_EX, 0
        );

        return key ? key : kb.VKey;
    }();

    key_ = [&] {
        if (vkey >= '0' && vkey <= '9') {
            //numbers
            return static_cast<keycode>(vkey);
        } else if (vkey >= 'A' && vkey <= 'Z') {
            //letters
            return static_cast<keycode>(vkey);
        } else if (!is_e0 && vkey >= VK_NUMPAD0 && vkey <= VK_NUMPAD9) {
            //numpad
            auto const x = vkey - VK_NUMPAD0;
            return static_cast<keycode>(static_cast<int>(keycode::NUM_0) + x);
        } else if (vkey >= VK_F1 && vkey <= VK_F24) {
            //f keys
            auto const x = vkey - VK_F1;
            return static_cast<keycode>(static_cast<int>(keycode::F1) + x);
        }

        switch (vkey) {
        case VK_LCONTROL : return keycode::CTRL_L;
        case VK_RCONTROL : return keycode::CTRL_R;
        case VK_LMENU    : return keycode::ALT_L;
        case VK_RMENU    : return keycode::ALT_R;
        case VK_LSHIFT   : return keycode::SHIFT_L;
        case VK_RSHIFT   : return keycode::SHIFT_R;

        case VK_CONTROL  : return is_e0 ? keycode::CTRL_R    : keycode::CTRL_L;
        case VK_MENU     : return is_e0 ? keycode::ALT_R     : keycode::ALT_L;
        case VK_SHIFT    : return is_e0 ? keycode::SHIFT_R   : keycode::SHIFT_L;
        case VK_RETURN   : return is_e0 ? keycode::NUM_ENTER : keycode::ENTER;
        case VK_INSERT   : return is_e0 ? keycode::INS       : keycode::NUM_0;
        case VK_DELETE   : return is_e0 ? keycode::DEL       : keycode::NUM_DEC;
        case VK_HOME     : return is_e0 ? keycode::HOME      : keycode::NUM_7;
        case VK_END      : return is_e0 ? keycode::END       : keycode::NUM_1;
        case VK_PRIOR    : return is_e0 ? keycode::PAGE_UP   : keycode::NUM_9;
        case VK_NEXT     : return is_e0 ? keycode::PAGE_DOWN : keycode::NUM_3;
        case VK_LEFT     : return is_e0 ? keycode::LEFT      : keycode::NUM_4;
        case VK_RIGHT    : return is_e0 ? keycode::RIGHT     : keycode::NUM_6;
        case VK_UP       : return is_e0 ? keycode::UP        : keycode::NUM_8;
        case VK_DOWN     : return is_e0 ? keycode::DOWN      : keycode::NUM_2;

        case VK_CLEAR    : BK_ASSERT(!is_e0); return keycode::NUM_5;

        case VK_NUMPAD2  : BK_ASSERT(is_e0); return keycode::DOWN;
        case VK_NUMPAD4  : BK_ASSERT(is_e0); return keycode::LEFT;
        case VK_NUMPAD6  : BK_ASSERT(is_e0); return keycode::RIGHT;
        case VK_NUMPAD8  : BK_ASSERT(is_e0); return keycode::UP;
        }

        return keycode::NONE;
    }();
}
//------------------------------------------------------------------------------
bklib::wstring_ref raw_keyboard::get_key_name(UINT scancode) {
    BK_ASSERT(!key_names.empty());
    BK_ASSERT(scancode < key_names.size());

    return key_names[scancode];
}
//------------------------------------------------------------------------------
