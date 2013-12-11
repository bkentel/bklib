//==============================================================================
//! @file
//! Include files for windows and windows libraries.
//==============================================================================
#pragma once

#include <memory>

#include <Windows.h>
#include <Unknwn.h>
#include <ObjIdl.h>

#include <d2d1.h>
#include <d2d1helper.h>
#pragma comment(lib, "D2d1.lib")

#include <dwrite.h>
#pragma comment(lib, "Dwrite.lib")

#include <wincodec.h>
#pragma comment(lib, "Windowscodecs.lib")

#include <imm.h>
#pragma comment(lib, "Imm32.lib")

#include <bklib/config.hpp>
#include <bklib/exception.hpp>
#include <bklib/assert.hpp>

namespace bklib {
namespace detail {
//==============================================================================
struct windows_error : virtual platform_error {};
struct com_error : virtual windows_error {};
//------------------------------------------------------------------------------
inline windows_error make_windows_error(char const* const function) {
    windows_error e;

    auto const error_code = ::GetLastError();
    auto const hr = HRESULT_FROM_WIN32(error_code);

    e << boost::errinfo_errno(hr)
      << boost::errinfo_api_function(function);
    return e;
}
//------------------------------------------------------------------------------
inline com_error make_com_error(char const* const function, HRESULT const hr) {
    BK_ASSERT(FAILED(hr));

    com_error e;
    e << boost::errinfo_errno(hr)
      << boost::errinfo_api_function(function);
    return e;
}
//------------------------------------------------------------------------------
struct hwnd_deleter {
    typedef HWND pointer;
    void operator()(pointer p) const BK_NOEXCEPT {
        ::DestroyWindow(p);
    }
};
//------------------------------------------------------------------------------
template <typename T>
struct com_deleter {
    static_assert(std::is_base_of<::IUnknown, T>::value,
        "Can only be used with COM types."
    );

    using pointer = T*;

    void operator()(pointer p) const BK_NOEXCEPT {
        if (p != nullptr) {
            p->Release();
        }
    }
};
//------------------------------------------------------------------------------
template <typename T>
using com_ptr = std::unique_ptr<T, com_deleter<T>>;
using window_handle = std::unique_ptr<HWND, hwnd_deleter>;

template <typename T>
inline com_ptr<T> make_com_ptr(T* ptr) {
    return com_ptr<T>(ptr);
}

template <typename T>
inline auto make_com_ptr(T&& function) {
    using result_t = std::remove_pointer_t<decltype(function())>;
    auto const ptr = function();
    BK_ASSERT(ptr != nullptr);
    return com_ptr<result_t>(ptr);
}

//==============================================================================
} //namespace detail
} //namespace bklib

#define BK_CHECK_COM(expression, api)\
[&] {\
    HRESULT const BK_CHECK_COM_result_value_ = expression;\
    if (SUCCEEDED(BK_CHECK_COM_result_value_)) return;\
    BOOST_THROW_EXCEPTION(::bklib::detail::com_error {}\
     << boost::errinfo_api_function(api)\
     << boost::errinfo_errno(BK_CHECK_COM_result_value_)\
    );\
}()

#define BK_COM_THROW_IF_FAILED(api, result)\
if (FAILED(result)) {\
    BOOST_THROW_EXCEPTION(::bklib::detail::com_error {}\
     << boost::errinfo_api_function(api)\
     << boost::errinfo_errno(result)\
    );\
} do {} while (FAILED(hr))
