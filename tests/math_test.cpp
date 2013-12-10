#include <gtest/gtest.h>
#include "math.hpp"

#define BK_STATIC_ASSERT_TYPE_EQ(TYPE, VAR)
//\
//    static_assert(\
//        ::std::is_same<\
//            TYPE\
//          , ::std::remove_cv<decltype(VAR)>::type\
//        >::value\
//      , "Unexpected type."\
//    )

TEST(Math, Vector) {
    using namespace bklib;

    vector2d<int> const ivec {3, -3};

    auto const mag  = magnitude(ivec);
    auto const dir  = direction<float>(ivec);
    auto const imag = magnitude<int>(dir);

    BK_STATIC_ASSERT_TYPE_EQ(float, mag);
    BK_STATIC_ASSERT_TYPE_EQ(float, dir.x);
    BK_STATIC_ASSERT_TYPE_EQ(int, imag);

    ASSERT_FLOAT_EQ(std::sqrt(18.0f), mag);   
    ASSERT_FLOAT_EQ(1.0f  / std::sqrt(2.0f), dir.x);
    ASSERT_FLOAT_EQ(-1.0f / std::sqrt(2.0f), dir.y);
    ASSERT_EQ(1, imag);
    ASSERT_FLOAT_EQ(1.0f, magnitude(dir));

    ASSERT_EQ(
        vector2d<float>({4.0f, -2.0f})
      , ivec + vector2d<float>({1.0f, 1.0f})
    );

    ASSERT_EQ(
        vector2d<int>({0, 0})
      , ivec + -ivec
    );
}

TEST(Math, Circle) {
    using namespace bklib;
    using point = point2d<int>;

    circle<int> c1 {{0, 0}, 10};
    circle<int> c2 {{0, 5}, 10};

    ASSERT_TRUE(intersects(c1, c2));
    ASSERT_EQ(distance(c1, c2), -15);
    ASSERT_EQ(distance(c1.p, c2.p), 5);

    auto const dist = distance<float>(c1, c2);
    ASSERT_EQ(dist, -15);

    auto const v = dist * direction<float>(c1.p - c2.p);
    c2.p += round_toward<int>(v);

    ASSERT_FLOAT_EQ(distance<float>(c1, c2), 0.0f);
    ASSERT_FALSE(intersects(c1, c2));

auto sv = separation_vector(c1, c2);
}

TEST(Math, BoundingCircle) {
    using namespace bklib;
    using rect = axis_aligned_rect<int>;

    auto const wa = 10;
    auto const ha = 5;
    auto const ra = rect{0, 0, 10, 5};

    auto const expected_rad_a = std::sqrt(square_of(wa / 2.0f) + square_of(ha / 2.0f));

    auto const ca_float = bounding_circle(ra);
    auto const ca_int   = bounding_circle<int>(ra);

    ASSERT_FLOAT_EQ(expected_rad_a, ca_float.r);
    ASSERT_GE(ca_int.r, ca_float.r);
}

//==============================================================================
// Basic tests for points and vectors and their interaction.
//==============================================================================
TEST(Math, PointAndVector) {
    using namespace bklib;

    point2d<int> const p1 {10, 10};
    point2d<int> const p2 {15, 16};

    ASSERT_EQ((p1 - p2), -(p2 - p1));
    ASSERT_EQ(dot(p1 - p2), dot(p2 - p1));
    ASSERT_EQ(p1 + (p2 - p1), p2);
    ASSERT_EQ(p2 + (p1 - p2), p1);

    ASSERT_EQ(magnitude(p1 - p2), magnitude(p2 - p1));
    ASSERT_EQ(direction<float>(p1 - p2), -direction<float>(p2 - p1));

    ASSERT_FLOAT_EQ(1.0f, magnitude(direction<float>(p1 - p2)));
    ASSERT_FLOAT_EQ(1.0f, magnitude(direction<float>(p2 - p1)));
}

//==============================================================================
// Sanity tests for rectangles.
//==============================================================================
TEST(Math, AARectIntersectionsSimple) {
    using namespace bklib;
    
    auto const x = 3;
    auto const y = 5;
    auto const w = 6;
    auto const h = 8;
    
    auto const tl = point2d<int>{x, y};
    auto const tr = point2d<int>{x + w, y};
    auto const bl = point2d<int>{x, y + h};
    auto const br = point2d<int>{x + w, y + h};

    using rect = axis_aligned_rect<int>;
    auto const r = rect{rect::tl_point{tl}, w, h};

    ASSERT_TRUE(r.is_well_formed());

    ASSERT_EQ(r.left(),   tl.x);
    ASSERT_EQ(r.top(),    tl.y);
    ASSERT_EQ(r.right(),  br.x);
    ASSERT_EQ(r.bottom(), br.y);

    ASSERT_EQ(tl, r.top_left());
    ASSERT_EQ(tr, r.top_right());
    ASSERT_EQ(bl, r.bottom_left());
    ASSERT_EQ(br, r.bottom_right());

    ASSERT_EQ(r.width(),  w);
    ASSERT_EQ(r.height(), h);

    ASSERT_EQ(w * h, area(r));
}

//==============================================================================
// Tests for valid intersections of rects.
//==============================================================================
TEST(Math, AARectIntersectionsValid) {
    using namespace bklib;

    using rect  = axis_aligned_rect<int>;
    using tl_point = rect::tl_point;

    rect const ra = {tl_point{0, 0}, 5, 5};

    rect const rb[] = {
        {tl_point{-1, -1}, 3, 3},
        {tl_point{ 1, -1}, 3, 3},
        {tl_point{ 3, -1}, 3, 3},
        {tl_point{-1,  1}, 3, 3},
        {tl_point{ 1,  1}, 3, 3},
        {tl_point{ 3,  1}, 3, 3},
        {tl_point{-1,  3}, 3, 3},
        {tl_point{ 1,  3}, 3, 3},
        {tl_point{ 3,  3}, 3, 3},
    };

    int const areas[] = {4, 6, 4, 6, 9, 6, 4, 6, 4};

    rect const rc[] = {
        {tl_point{0, 0}, 2, 2},
        {tl_point{1, 0}, 3, 2},
        {tl_point{3, 0}, 2, 2},
        {tl_point{0, 1}, 2, 3},
        {tl_point{1, 1}, 3, 3},
        {tl_point{3, 1}, 2, 3},
        {tl_point{0, 3}, 2, 2},
        {tl_point{1, 3}, 3, 2},
        {tl_point{3, 3}, 2, 2},
    };

    for (int i = 0; i < 9; ++i) {
        ASSERT_TRUE(intersects(ra, rb[i]));
        auto const ir = intersection_of(ra, rb[i]);
        ASSERT_FALSE(!ir);
        ASSERT_TRUE(ir.result.is_well_formed());
        ASSERT_EQ(areas[i], area(ir.result));
        ASSERT_EQ(rc[i], ir.result);
    }
}

//==============================================================================
// Tests for invalid intersections of rects.
//==============================================================================
TEST(Math, AARectIntersectionsInvalid) {
    using namespace bklib;

    using rect  = axis_aligned_rect<int>;
    using tl_point = rect::tl_point;

    auto const r  = rect{tl_point{0, 0}, 10, 10};
    rect const r1 = {10, 1,  11, 11};
    rect const r2 = {1,  10, 11, 11};
    rect const r3 = {10, 10, 11, 11};

    ASSERT_FALSE(!!intersection_of(r, r1));
    ASSERT_FALSE(!!intersection_of(r, r2));
    ASSERT_FALSE(!!intersection_of(r, r3));

    ASSERT_FALSE(intersection_of(r, r1).result.is_well_formed());
    ASSERT_FALSE(intersection_of(r, r2).result.is_well_formed());
    ASSERT_FALSE(intersection_of(r, r3).result.is_well_formed());

    ASSERT_FALSE(intersects(r, r1));
    ASSERT_FALSE(intersects(r, r2));
    ASSERT_FALSE(intersects(r, r3));
}
