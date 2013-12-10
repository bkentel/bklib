#pragma once

#include "macros.hpp"

namespace bklib {

template <typename F>
struct scope_exit {
    scope_exit(F f) : f {f}, canceled {false} {}
    ~scope_exit() { if (!canceled) f(); }

    void cancel() {
        canceled = true;
    }

    F f;
    bool canceled;
};

template <typename F>
scope_exit<F> make_scope_exit(F f) {
    return scope_exit<F>(f);
};

} //bklib

#define BK_SCOPE_EXIT(CODE) \
auto BK_UNIQUE_ID(scope_exit_) = ::bklib::make_scope_exit([&]CODE)

#define BK_SCOPE_EXIT_NAME(NAME, CODE) \
auto NAME = ::bklib::make_scope_exit([&]CODE)
