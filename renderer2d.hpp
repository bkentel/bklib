#pragma once

#include "math.hpp"
#include "types.hpp"
#include "window.hpp"

namespace bklib {

namespace gfx {
    template <typename Tag, typename T, size_t N>
    struct tagged_array {
        using tag = Tag;
        std::array<T, N> values;
    };

    template <typename Tag, size_t I>
    struct tagged_index {
        using tag = Tag;
        static size_t const index = I;
    };

    namespace detail {
        struct tag_color;
    } //namespace detail

    template <typename Tag, typename T, size_t I>
    struct tagged_value_index : public tagged_index<Tag, I> {
        using type = T;

        explicit tagged_value_index(T const value) : value{value} {}

        operator T() const BK_NOEXCEPT { return value; }

        T value;
    };

    template <typename T, size_t N>
    struct color {
        std::array<T, N> values;
    };

    template <typename T, size_t I>
    struct color_index : public tagged_value_index<detail::tag_color, T, I> {
        template <size_t N>
        color_index(color<T, N> const& c) : tagged_value_index{c.values[I]} {
            static_assert(I < N, "size mismatch");
        }
    };

    template <typename T> using red_t   = color_index<T, 0>;
    template <typename T> using green_t = color_index<T, 1>;
    template <typename T> using blue_t  = color_index<T, 2>;
    template <typename T> using alpha_t = color_index<T, 3>;

    template <typename T, size_t N>
    inline red_t<T> red(color<T, N> const& c) { return red_t {c}; }

    template <typename T, size_t N>
    inline green_t<T> green(color<T, N> const& c) { return green_t {c}; }

    template <typename T, size_t N>
    inline blue_t<T> blue(color<T, N> const& c) { return blue_t {c}; }

    template <typename T, size_t N>
    inline alpha_t<T> alpha(color<T, N> const& c) { return green_t {c}; }

    using color3f = color<float, 3>;
} //namespace gfx

class renderer2d {
public:
    using coord_t = float;
    using rect    = bklib::axis_aligned_rect<coord_t>;
    using point   = bklib::point2d<coord_t>;

    using color       = gfx::color<float, 3>;
    using color_alpha = gfx::color<float, 4>;

    template <typename T>
    inline static rect convert_rect(bklib::axis_aligned_rect<T> const in) {
        return {
            static_cast<coord_t>(in.left())
          , static_cast<coord_t>(in.top())
          , static_cast<coord_t>(in.right())
          , static_cast<coord_t>(in.bottom())
        };
    }

    explicit renderer2d(bklib::platform_window_handle handle);
    ~renderer2d();

    void resize(unsigned width, unsigned height);

    void begin();
    void end();

    void clear() {
        color_alpha const clear_color {1.0f, 0.0f, 0.0f, 1.0f};
        clear(clear_color);
    }

    void clear(color c) {
        color_alpha const clear_color {gfx::red(c), gfx::green(c), gfx::blue(c), 1.0f};
        clear(clear_color);
    }

    void clear(color_alpha c);

    void draw_text(bklib::string_ref  text);
    void draw_text(bklib::wstring_ref text);

    void draw_rect(rect r, coord_t width = 1.0f);

    template <typename T, typename std::enable_if<!std::is_same<T, coord_t>::value>::type* = 0>
    void draw_rect(bklib::axis_aligned_rect<T> const r, coord_t const width = 1.0f) {
        draw_rect(convert_rect(r), width);
    }

    void draw_round_rect();

    void fill_rect(rect const r);

    template <typename T, typename std::enable_if<!std::is_same<T, coord_t>::value>::type* = 0>
    void fill_rect(bklib::axis_aligned_rect<T> const r) {
        fill_rect(convert_rect(r));
    }

    void fill_round_rect();

    void set_color_brush(color c) {
        color_alpha const c1 {gfx::red(c), gfx::green(c), gfx::blue(c), 1.0f};
        set_color_brush(c1);
    }

    void set_color_brush(color_alpha c);

    class impl_t;
private:
    std::unique_ptr<impl_t> impl_;
};

} //namespace bklib
