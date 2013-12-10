#pragma once

#include <functional>
#include <type_traits>

#include "macros.hpp"

namespace bklib {

template <typename Tag, typename Signature>
class callback {
public:
    using function_type = std::function<Signature>;
    using result_type   = typename function_type::result_type;

    static_assert(std::is_void<result_type>::value, "TODO: must return void");

    BK_DEFAULT_ALL(callback);
    callback() = default;

    //--------------------------------------------------------------------------
    template <typename F>
    callback(F&& function)
      : value{std::forward<F>(function)}
    {
    }
    //--------------------------------------------------------------------------
    template <typename F>
    callback& operator=(F&& function) {
        value = std::forward<F>(function);
        return *this;
    }
    //--------------------------------------------------------------------------
    void swap(callback& other) BK_NOEXCEPT {
        using std::swap;
        swap(value, other.value);
    }
    //--------------------------------------------------------------------------
    template <typename... Args>
    void operator()(Args&&... args) const {
        value(std::forward<Args>(args)...);
    }
    //--------------------------------------------------------------------------
    explicit operator bool() const BK_NOEXCEPT {
        return !!value;
    }
private:
    std::function<Signature> value;
};

} //namespace bklib

#define BK_DECLARE_EVENT(name, sig)\
namespace detail {\
    struct tag_##name;\
}\
using name = ::bklib::callback<detail::tag_##name, sig>
