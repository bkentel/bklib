//==============================================================================
//! Mathematical functions and objects.
//! @file
//! @author Brandon Kentel
//==============================================================================
#pragma once

#include "config.hpp"

namespace bklib {

//==============================================================================
//! Tracked simultaneous min/max value.
//==============================================================================
template <typename T>
struct min_max {
    explicit min_max(T const initial = T{0}) BK_NOEXCEPT
      : min{initial}, max{initial}
    {
    }

    void operator()(T const n) BK_NOEXCEPT {
        if (n < min) min = n;
        else if (n > max) max = n;
    }

    T range() const BK_NOEXCEPT { return max - min; }

    T min, max;
};

//==============================================================================
//! Convenience type for std::common_type<T, U>.
//==============================================================================
template <typename T, typename U>
using common_type_t = typename std::common_type<T, U>::type;

//==============================================================================
//! Enable if @c Test is floating point.
//==============================================================================
template <typename Test, typename Type = void>
using enable_for_floating_point_t = typename std::enable_if<
    std::is_floating_point<Test>::value, Type
>::type;

//==============================================================================
//! Enable if @c Test is Integral.
//==============================================================================
template <typename Test, typename Type = void>
using enable_for_integral_t = typename std::enable_if<
    std::is_integral<Test>::value, Type
>::type;

//==============================================================================
//! @see if_not_void_t.
//==============================================================================
template <typename Test, typename Default>
struct if_not_void : public std::conditional<
    std::is_void<Test>::value, Default, Test
> { };
//==============================================================================
//! @see if_not_void_common_t.
//==============================================================================
template <typename Test, typename T1, typename T2>
struct if_not_void_common : public std::conditional<
    std::is_void<Test>::value, typename std::common_type<T1, T2>::type, Test
> { };
//==============================================================================
//! If @c Test is @c void then @c Default, otherwise @c Test.
//! @tparam Test Type to test against void.
//! @tparam Default Type to use when @c Test is void.
//==============================================================================
template <typename Test, typename Default>
using if_not_void_t = typename if_not_void<Test, Default>::type;

//==============================================================================
//! If @c Test is @c void then @c common_type<T1, T2>, otherwise @c Test.
//! @tparam Test type to test against void.
//! @tparam T1 First type.
//! @tparam T2 Second type.
//==============================================================================
template <typename Test, typename T1, typename T2>
using if_not_void_common_t = typename if_not_void_common<Test, T1, T2>::type;

//==============================================================================
//! @returns One of (-1, 0, 1) corresponding to the sign of @c x.
//==============================================================================
template <typename T, enable_for_integral_t<T>* = 0>
T sign_of(T const x) {
    return (x > T{0}) - (x < T{0});
}
//------------------------------------------------------------------------------
template <typename T, enable_for_floating_point_t<T>* = 0>
T sign_of(T const x) {
    //TODO
    return (x > T{0}) - (x < T{0});
}
//==============================================================================
//! Unified template for fixed size integer types.
//! @tparam N The number of bytes for the integer type.
//! @tparam Signed Whether the integer type should be signed.
//==============================================================================
template <unsigned N, bool Signed = true>
struct sized_integral;

template <> struct sized_integral<1, true> { using type = std::int8_t;  };
template <> struct sized_integral<2, true> { using type = std::int16_t; };
template <> struct sized_integral<4, true> { using type = std::int32_t; };
template <> struct sized_integral<8, true> { using type = std::int64_t; };

template <> struct sized_integral<1, false> { using type = std::uint8_t;  };
template <> struct sized_integral<2, false> { using type = std::uint16_t; };
template <> struct sized_integral<4, false> { using type = std::uint32_t; };
template <> struct sized_integral<8, false> { using type = std::uint64_t; };
//------------------------------------------------------------------------------
//! @see sized_integral.
//------------------------------------------------------------------------------
template <unsigned N>
using sized_signed_t = typename sized_integral<N, true>::type;
//------------------------------------------------------------------------------
//! @see sized_integral.
//------------------------------------------------------------------------------
template <unsigned N>
using sized_unsigned_t = typename sized_integral<N, false>::type;
//==============================================================================
//! Equality comparison for floating point, otherwise use built in comparison.
//==============================================================================
template <typename T, enable_for_floating_point_t<T>* = 0>
bool is_equal(T const a, T const b, int const ulps = 4) {
    BK_ASSERT(ulps > 0);

    using int_t = sized_signed_t<sizeof(T)>;

    static auto const max = std::numeric_limits<int_t>::max() + 1;

    auto const ai = *reinterpret_cast<int_t const*>(&a);
    auto const bi = *reinterpret_cast<int_t const*>(&b);

    auto const an = ai < 0 ? max - ai : ai;
    auto const bn = bi < 0 ? max - bi : bi;

    return std::abs(an - bn) <= ulps;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
bool is_equal(T const a, U const b) {
    return a == b;
}

//==============================================================================
//! @returns The square of x.
//==============================================================================
template <typename T>
T square_of(T const x) { return x*x; }

//==============================================================================
//! 2D point type.
//==============================================================================
template <typename T>
struct point2d { T x; T y; };
//==============================================================================
//! 2D vector type.
//==============================================================================
template <typename T>
struct vector2d { T x; T y; };
//==============================================================================
//! Circle type.
//==============================================================================
template <typename T>
struct circle {
    point2d<T> p; //!<< Center.
    T          r; //<<! Radius.
};

namespace detail {
    //!Types common to all axis_aligned_rect.
    struct axis_aligned_rect_base {
        //!Tag type to flag the constructor as allowing malformed rectangles.
        struct allow_malformed {};

        //!Tag type to differentiate points by their vertex.
        template <typename T, typename Tag>
        struct tagged_point {
            tagged_point(T x, T y) : value{{x, y}} {}
            tagged_point(point2d<T> p) : value{{p.x, p.y}} {}

            operator point2d<T>&() { return value; }
            operator point2d<T> const&() const { return value; }

            point2d<T> value;
        };

        struct tag_tl_point;     //!<< top left.
        struct tag_tr_point;     //!<< top right.
        struct tag_bl_point;     //!<< bottom left.
        struct tag_br_point;     //!<< bottom right.
        struct tag_center_point; //!<< center.
    };
} //namespace detail

//==============================================================================
//! Axis-aligned rectangle type.
//==============================================================================
template <typename T>
class axis_aligned_rect : public detail::axis_aligned_rect_base {
public:
    using point  = point2d<T>;
    using vector = vector2d<T>;

    using tl_point     = tagged_point<T, tag_tl_point>;
    using tr_point     = tagged_point<T, tag_tr_point>;
    using bl_point     = tagged_point<T, tag_bl_point>;
    using br_point     = tagged_point<T, tag_br_point>;
    using center_point = tagged_point<T, tag_center_point>;
    //--------------------------------------------------------------------------
    //! Unchecked construction from {x0, y0, x1, y1} = {left, top, right, bottom}.
    //--------------------------------------------------------------------------
    axis_aligned_rect(allow_malformed, T x0, T y0, T x1, T y1) BK_NOEXCEPT
      : x0_{x0}, y0_{y0}, x1_{x1}, y1_{y1}
    {
    }
    //--------------------------------------------------------------------------
    //! Checked construction from {x0, y0, x1, y1} = {left, top, right, bottom}.
    //! @pre (x1 > x0) && (y1 > y0).
    //! @post is_well_formed() == true.
    //--------------------------------------------------------------------------
    axis_aligned_rect(T x0, T y0, T x1, T y1) BK_NOEXCEPT
      : axis_aligned_rect(allow_malformed{}, x0, y0, x1, y1)
    {
        BK_ASSERT(x1 > x0);
        BK_ASSERT(y1 > y0);
    }
    //--------------------------------------------------------------------------
    //! Default construction to {left, top, right, bottom} = {0, 0, 0, 0}.
    //! @post is_well_formed() == false.
    //--------------------------------------------------------------------------
    axis_aligned_rect() BK_NOEXCEPT
      : axis_aligned_rect(allow_malformed{}, T{0}, T{0}, T{0}, T{0})
    {
    }
    //--------------------------------------------------------------------------
    //! List construction.
    //! @pre list.size() == 4.
    //! @post is_well_formed() == true.
    //--------------------------------------------------------------------------
    axis_aligned_rect(std::initializer_list<T> list) BK_NOEXCEPT
        : axis_aligned_rect(
            *(list.begin() + 0)
          , *(list.begin() + 1)
          , *(list.begin() + 2)
          , *(list.begin() + 3)
        )
    {
        BK_ASSERT(list.size() == 4);
    }
    //--------------------------------------------------------------------------
    //! Construct from top left point and dimensions.
    //! @pre (w > 0) && (h > 0)
    //! @post is_well_formed() == true.
    //--------------------------------------------------------------------------
    axis_aligned_rect(tl_point p, T w, T h) BK_NOEXCEPT
      : axis_aligned_rect(p.value, w, h)
    {
    }
    //--------------------------------------------------------------------------
    //! Construct from top center point and dimensions.
    //! @pre (w > 0) && (h > 0)
    //! @post is_well_formed() == true.
    //--------------------------------------------------------------------------
    axis_aligned_rect(center_point p, T w, T h) BK_NOEXCEPT
      : axis_aligned_rect(p.value - vector2d<T>({w / T{2}, h / T{2}}), w, h)
    {
    }
    //--------------------------------------------------------------------------
    //! @return Width of the rectangle.
    //! @pre is_well_formed() == true.
    //! @post result > 0.
    //--------------------------------------------------------------------------
    T width() const BK_NOEXCEPT { return right()  - left(); }
    //--------------------------------------------------------------------------
    //! @return Height of the rectangle.
    //! @pre is_well_formed() == true.
    //! @post result > 0.
    //--------------------------------------------------------------------------
    T height() const BK_NOEXCEPT { return bottom() - top(); }

    T left()   const BK_NOEXCEPT { return x0_; }
    T right()  const BK_NOEXCEPT { return x1_; }
    T top()    const BK_NOEXCEPT { return y0_; }
    T bottom() const BK_NOEXCEPT { return y1_; }

    point top_left()     const BK_NOEXCEPT { return {left(),  top()}; }
    point top_right()    const BK_NOEXCEPT { return {right(), top()}; }
    point bottom_left()  const BK_NOEXCEPT { return {left(),  bottom()}; }
    point bottom_right() const BK_NOEXCEPT { return {right(), bottom()}; }
    //--------------------------------------------------------------------------
    //! @return The center point of the rectangle.
    //! @tparam R The element type for the return value; defaulted to @c T.
    //! @pre is_well_formed() == true.
    //--------------------------------------------------------------------------
    template <typename R = T>
    point2d<R> center() const BK_NOEXCEPT {
        R const x = left() + width()  / R{2};
        R const y = top()  + height() / R{2};
        return {x, y};
    }

    bool is_well_formed() const BK_NOEXCEPT {
        return (left() < right()) && (top() < bottom());
    }
private:
    //--------------------------------------------------------------------------
    //! Point plus dimensions construction.
    //! @pre (w > 0) && (h > 0);
    //! @post is_well_formed() == true.
    //--------------------------------------------------------------------------
    axis_aligned_rect(point p, T w, T h) BK_NOEXCEPT
        : axis_aligned_rect(allow_malformed{}, p.x, p.y, p.x + w, p.y + h)
    {
        BK_ASSERT(w > T{0});
        BK_ASSERT(h > T{0});
    }

    T x0_;
    T y0_;
    T x1_;
    T y1_;
};
//==============================================================================
//! Result type for geometric intersections.
//==============================================================================
template <typename T>
struct intersection_result {
    bool valid;  //!<< Whether the objects intersected.
    T    result; //!<< The result of the intersection.

    explicit operator bool() const BK_NOEXCEPT { return valid; }
};
//==============================================================================
//! The geometric union of n objects.
//==============================================================================
template <typename T, typename... Types>
struct geometric_union {
    using point = point2d<T>;
    
    //Recursive case
    template <int I = -1>
    bool intersects(point const p) const {
        return bklib::intersects(std::get<I-1>(types), p) && intersects<I-2>(p);
    }

    //Default case
    template <>
    bool intersects<-1>(point const p) const {
        return intersects<sizeof...(Types)>(p);
    }

    //Base case
    template <>
    bool intersects<0>(point const p) const {
        return bklib::intersects(std::get<0>(types), p);
    }

    std::tuple<Types...> types;
};
//------------------------------------------------------------------------------
// Base case
//------------------------------------------------------------------------------
template <typename T>
struct geometric_union<T> {
    using point = point2d<T>;

    template <int I = -1>
    bool intersects(point const p) const {
        return true;
    }
};

template <typename T, typename... Types>
intersection_result<point2d<T>> intersects(
    geometric_union<Types...> const u,
    point2d<T> const p
) {
    return {u.intersects(p), p};
}
//==============================================================================
// Construction helpers.
//==============================================================================
template <typename T, typename U>
inline auto make_point2d(T const x, U const y) BK_NOEXCEPT {
    using type = common_type_t<T, U>;
    point2d<type> result = {static_cast<type>(x), static_cast<type>(y)};
    return result;
}

template <typename T, typename U>
inline auto make_vector2d(T x, U y) BK_NOEXCEPT {
    using type = common_type_t<T, U>;
    vector2d<type> result = {static_cast<type>(x), static_cast<type>(y)};
    return result;
}

//==============================================================================
// Global built-in operators.
//==============================================================================
template <typename T>
bool operator==(
    axis_aligned_rect<T> const lhs
  , axis_aligned_rect<T> const rhs
) BK_NOEXCEPT {
    return is_equal(lhs.left(),   rhs.left())
        && is_equal(lhs.right(),  rhs.right())
        && is_equal(lhs.top(),    rhs.top())
        && is_equal(lhs.bottom(), rhs.bottom());
}
//------------------------------------------------------------------------------
template <typename T>
bool operator!=(
    axis_aligned_rect<T> const lhs
  , axis_aligned_rect<T> const rhs
) BK_NOEXCEPT {
    return !(lhs == rhs);
}
//------------------------------------------------------------------------------
template <typename T>
bool operator==(point2d<T> const lhs, point2d<T> const rhs) BK_NOEXCEPT {
    return is_equal(lhs.x, rhs.x)
        && is_equal(lhs.y, rhs.y);
}
//------------------------------------------------------------------------------
template <typename T>
bool operator!=(point2d<T> const lhs, point2d<T> const rhs) BK_NOEXCEPT {
    return !(lhs == rhs);
}
//------------------------------------------------------------------------------
template <typename T>
bool operator==(vector2d<T> const lhs, vector2d<T> const rhs) BK_NOEXCEPT {
    return is_equal(lhs.x, rhs.x)
        && is_equal(lhs.y, rhs.y);
}
//------------------------------------------------------------------------------
template <typename T>
bool operator!=(vector2d<T> const lhs, vector2d<T> const rhs) BK_NOEXCEPT {
    return !(lhs == rhs);
}
//------------------------------------------------------------------------------
template <typename T>
auto operator-(vector2d<T> const v) BK_NOEXCEPT {
    return vector2d<T>{-v.x, -v.y};
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator*(vector2d<T> const v, U const c) BK_NOEXCEPT {
    using type = vector2d<common_type_t<T, U>>;
    return type{c * v.x, c * v.y};
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator*(T const c, vector2d<U> const v) BK_NOEXCEPT {
    return v * c;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator/(vector2d<T> const v, U const c) BK_NOEXCEPT {
    vector2d<common_type_t<T, U>> result = {v.x / c, v.y / c};
    return result;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator/(U const c, vector2d<T> const v) BK_NOEXCEPT {
    return v / c;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
vector2d<common_type_t<T, U>> operator+(vector2d<T> const u, vector2d<U> const v) BK_NOEXCEPT {
    return make_vector2d(u.x + v.x, u.y + v.y);

//    vector2d<common_type_t<T, U>> result = {u.x + v.x, u.y + v.y};
    //return result;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator+(point2d<T> const p, vector2d<U> const v) BK_NOEXCEPT {
    point2d<common_type_t<T, U>> result = {p.x + v.x, p.y + v.y};
    return result;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator+(axis_aligned_rect<T> const r, vector2d<U> const v) BK_NOEXCEPT {
    using type = axis_aligned_rect<common_type_t<T, U>>;
    type::tl_point const p = r.top_left() + v;
    return type{p, r.width(), r.height()};
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator-(vector2d<T> const u, vector2d<U> const v) BK_NOEXCEPT {
    using type = vector2d<common_type_t<T, U>>;
    return type{u.x - v.x, u.y - v.y};
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator-(point2d<T> const p, vector2d<U> const v) BK_NOEXCEPT {
    using type = point2d<common_type_t<T, U>>;
    return type{p.x - v.x, p.y - v.y};
}
//------------------------------------------------------------------------------
template <typename T, typename U>
vector2d<common_type_t<T, U>> operator-(point2d<T> const p, point2d<U> const q) BK_NOEXCEPT {
    vector2d<common_type_t<T, U>> result = {p.x - q.x, p.y - q.y};
    return result;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
auto operator-(axis_aligned_rect<T> const r, vector2d<U> const v) BK_NOEXCEPT {
    using type = axis_aligned_rect<common_type_t<T, U>>;
    type::tl_point const p = r.top_left() - v;

    type result{p, r.width(), r.height()};

    return result;
}
//------------------------------------------------------------------------------
template <typename T, typename U>
T& operator+=(T& lhs, vector2d<U> const rhs) BK_NOEXCEPT {
    return (lhs = lhs + rhs);
}

template <typename T, typename U>
T& operator-=(T& lhs, vector2d<U> const rhs) BK_NOEXCEPT {
    return (lhs = lhs - rhs);
}

//==============================================================================
// Other operators.
//==============================================================================
template <typename R = void, typename T = void, typename U = void>
auto dot(vector2d<T> const u, vector2d<U> const v) BK_NOEXCEPT {
    using type = if_not_void_common_t<R, T, U>;
    return static_cast<type>(u.x * v.x + u.y * v.y);
}
//------------------------------------------------------------------------------
template <typename R = void, typename T = void>
auto dot(vector2d<T> const v) BK_NOEXCEPT {
    using type = if_not_void_t<R, T>;
    return static_cast<type>(dot(v, v));
}
//------------------------------------------------------------------------------

//==============================================================================
//! If T is a floating point type, then T, otherwise Default.
//==============================================================================
template <typename T, typename Default = float>
using floating_point_t = typename std::conditional<
    std::is_floating_point<T>::value, T, Default
>::type;

//==============================================================================
//! @returns The scalar magnitude of the vector @c v as @c R or a floating
//!          point type.
//! @tparam R The result type if not @c void.
//==============================================================================
template <typename R = void, typename T>
inline auto magnitude(vector2d<T> const v) BK_NOEXCEPT {
    using type = if_not_void_t<R, floating_point_t<T>>;
    return static_cast<type>(std::sqrt(dot(v)));
}
//==============================================================================
//! @returns A unit vector in the direction of v.
//! @tparam R The element type if not @c void.
//==============================================================================
template <typename R = void, typename T>
inline auto direction(vector2d<T> const v) BK_NOEXCEPT {
    using type = if_not_void_t<R, floating_point_t<T>>;
    type const mag = magnitude<type>(v);

    vector2d<type> result = is_equal(mag, type{0}) ? to_type<type>(v) : (v / mag);

    return result;
}

//------------------------------------------------------------------------------
template <typename T>
T area(axis_aligned_rect<T> const r) BK_NOEXCEPT {
    return r.width() * r.height();
}
//==============================================================================
// Conversions.
//==============================================================================
template <typename R, typename T>
vector2d<R> to_type(vector2d<T> const v) BK_NOEXCEPT {
    return {static_cast<R>(v.x), static_cast<R>(v.y)};
}

template <typename R, typename T>
point2d<R> to_type(point2d<T> const p) BK_NOEXCEPT {
    return {static_cast<R>(p.x), static_cast<R>(p.y)};
}

//==============================================================================
// Distances
//==============================================================================
//------------------------------------------------------------------------------
// point <-> point
//------------------------------------------------------------------------------
template <typename T, typename U>
auto distance2(point2d<T> const a, point2d<U> const b) BK_NOEXCEPT {
    return dot(a - b);
}
//------------------------------------------------------------------------------
template <typename R = void, typename T = void, typename U = void>
auto distance(point2d<T> const a, point2d<U> const b) BK_NOEXCEPT {
    using type = if_not_void_t<R, common_type_t<T, U>>;

    return static_cast<type>(
        std::sqrt(distance2(a, b))
    );
}
//------------------------------------------------------------------------------
// circle <-> circle
//------------------------------------------------------------------------------
template <typename R = void, typename T = void, typename U = void>
auto distance(circle<T> const a, circle<U> const b) BK_NOEXCEPT {
    using type = if_not_void_t<R, T>;

    return static_cast<type>(
        std::sqrt(dot(a.p - b.p)) - a.r - b.r
    );
}
//------------------------------------------------------------------------------
// rectangle <-> rectangle
//------------------------------------------------------------------------------
template <
    typename CornerA = axis_aligned_rect<T>::center_t
  , typename CornerB = axis_aligned_rect<T>::center_t
  , typename T = void
>
T distance2(
    axis_aligned_rect<T> const ra
  , axis_aligned_rect<T> const rb
  , CornerA = CornerA{}
  , CornerB = CornerB{}
) BK_NOEXCEPT {
    auto const a = ra.get_point(CornerA{});
    auto const b = ra.get_point(CornerB{});

    return dot(a - b);
}
////////////////////////////////////////////////////////////////////////////////
// Boolean intersections.
////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//! circle <-> point intersection.
//==============================================================================
template <typename T>
bool intersects(circle<T> const c, point2d<T> const p) BK_NOEXCEPT {
    return distance2(p, c.p) < square_of(c.r);
}
//==============================================================================
//! point <-> circle intersection.
//==============================================================================
template <typename T>
bool intersects(point2d<T> const p, circle<T> const  c) BK_NOEXCEPT {
    return intersects(c, p);
}
//==============================================================================
//! axis_aligned_rect <-> circle intersection.
//==============================================================================
template <typename T>
bool intersects(axis_aligned_rect<T> const r, circle<T> const c) BK_NOEXCEPT {
    return intersects(bounding_circle<T>(r), c) && (
        intersects(c, r.top_left())
     || intersects(c, r.top_right())
     || intersects(c, r.bottom_left())
     || intersects(c, r.bottom_right())
    );
}
//==============================================================================
//! circle <-> axis_aligned_rect intersection.
//==============================================================================
template <typename T>
bool intersects(circle<T> const c, axis_aligned_rect<T> const r) BK_NOEXCEPT {
    return intersects(r, c);
}
//==============================================================================
//! axis_aligned_rect <-> axis_aligned_rect intersection.
//==============================================================================
template <typename T>
bool intersects(axis_aligned_rect<T> const ra, axis_aligned_rect<T> const rb) BK_NOEXCEPT {
    return !(
        ra.right()  <= rb.left()
     || ra.bottom() <= rb.top()
     || ra.left()   >= rb.right()
     || ra.top()    >= rb.bottom()
    );
}
//==============================================================================
//! axis_aligned_rect <-> point intersection.
//==============================================================================
template <typename T>
bool intersects(axis_aligned_rect<T> const r, point2d<T> const p) BK_NOEXCEPT {
    return !(
        p.x < r.left() || p.x >= r.right()
     || p.y < r.top()  || p.y >= r.bottom()
    );
}
//==============================================================================
//! point <-> axis_aligned_rect intersection.
//==============================================================================
template <typename T>
bool intersects(point2d<T> const p, axis_aligned_rect<T> const r) BK_NOEXCEPT {
    return intersects(r, p);
}
//==============================================================================
//! circle <-> circle intersection.
//==============================================================================
template <typename T>
bool intersects(circle<T> const a, circle<T> const b) BK_NOEXCEPT {
    auto const dist = distance2(a.p, b.p);
    auto const r    = square_of(a.r + b.r);

    return dist < r;
}
////////////////////////////////////////////////////////////////////////////////
// Geometric intersections.
////////////////////////////////////////////////////////////////////////////////

//==============================================================================
//! axis_aligned_rect <-> axis_aligned_rect intersection result.
//==============================================================================
template <typename T>
auto intersection_of(axis_aligned_rect<T> const ra, axis_aligned_rect<T> const rb) BK_NOEXCEPT {
    auto const l = std::max(ra.left(),   rb.left());
    auto const r = std::min(ra.right(),  rb.right());
    auto const t = std::max(ra.top(),    rb.top());
    auto const b = std::min(ra.bottom(), rb.bottom());

    using rect = axis_aligned_rect<T>;

    return intersection_result<rect>{{
        (l < r) && (t < b)
      , {rect::allow_malformed{}, l, t, r, b}
    }};   
}
template <typename T>
auto intersection_of(axis_aligned_rect<T> const r, point2d<T> const p) BK_NOEXCEPT {
    using type = intersection_result<point2d<T>>;
    return type{intersects{r, p}, p};
}
//------------------------------------------------------------------------------
template <typename T>
auto intersection_of(point2d<T> const p, axis_aligned_rect<T> const r) BK_NOEXCEPT {
    return intersection_of(r, p);
}
//------------------------------------------------------------------------------


//==============================================================================
//! Generate a random direction vector.
//==============================================================================
template <typename T = float, typename F = void>
vector2d<T> random_direction(F& random) BK_NOEXCEPT {
    static float const pi2 = 2.0f * std::acos(0.0f);

    auto const angle = std::uniform_real_distribution<float>(0.0f, pi2)(random);

    auto const x = static_cast<T>(std::sin(angle));
    auto const y = static_cast<T>(std::cos(angle));

    return {x, y};
}

//==============================================================================
//! Special rounding.
//!
//! Source   | Result   | Action
//! ---------|----------|-------------------------------------------------------
//! Floating | Integral | Round to next most negative / positive integer
//! Floating | Floating | Type cast
//! Integral | Integral | Type cast
//! Integral | Floating | Type cast
//==============================================================================
template <typename Result, typename Source, 
    typename std::enable_if<
        std::is_integral<Result>::value
     && std::is_floating_point<Source>::value
    >::type* = 0
>
Result round_up_if_integral(Source const x) BK_NOEXCEPT {
    return (x >= Source{0})
      ? static_cast<Result>(std::ceil(x))
      : static_cast<Result>(std::floor(x));
}
//------------------------------------------------------------------------------
template <typename Result, typename Source,
    typename std::enable_if<!(
        std::is_integral<Result>::value
     && std::is_floating_point<Source>::value
    )>::type* = 0
>
Result round_up_if_integral(Source const x) BK_NOEXCEPT {
    return static_cast<Result>(x);
}

//==============================================================================
//! Return the circle that rect can be inscribed in.
//==============================================================================
template <typename R = float, typename T = void>
circle<R> bounding_circle(axis_aligned_rect<T> const rect) BK_NOEXCEPT {
    auto const p = rect.center<float>();
    auto const q = to_type<float>(rect.top_left());
    auto const r = round_up_if_integral<R>(distance(p, q));

    return {to_type<R>(p), r};
}

//==============================================================================
//! Rounds @c n to the next most negative / positive integer value.
//==============================================================================
template <typename T
  , enable_for_floating_point_t<T>* = 0 //floating point types.
>
T round_toward(T const n) BK_NOEXCEPT {
    return n >= T{0} ? std::ceil(n) : std::floor(n);
}
//------------------------------------------------------------------------------
template <typename T
  , enable_for_integral_t<T>* = 0 //integral types.
>
T round_toward(T const n) BK_NOEXCEPT {
    return n;
}
//==============================================================================
//! Rounds the vector @c v in its direction.
//==============================================================================
template <typename R, typename T>
vector2d<R> round_toward(vector2d<T> const v) BK_NOEXCEPT {
    auto const x = static_cast<R>(round_toward(v.x));
    auto const y = static_cast<R>(round_toward(v.y));

    return {x, y};
}
//==============================================================================
//! Rounds the point @c p in the direction of @c v.
//==============================================================================
template <typename R, typename T, typename U>
point2d<R> round_toward(point2d<T> const p, vector2d<U> const v) BK_NOEXCEPT {
    auto const x = static_cast<R>(round_toward(p.x + v.x));
    auto const y = static_cast<R>(round_toward(p.y + v.y));

    return {x, y};
}

//==============================================================================
//! Return a vector between the centers of @c a and @c b whose magnitude is the
//! distance between their perimeters.
//==============================================================================
template <typename T, typename U>
vector2d<float> separation_vector(bklib::circle<T> const a, bklib::circle<U> const b) BK_NOEXCEPT {
    auto const dir = bklib::direction<float>(a.p - b.p);
    auto const mag = -bklib::distance<float>(a, b);
    return mag * dir;
};

} //namespace bklib
