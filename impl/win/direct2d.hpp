#pragma once

#include <type_traits>
#include <memory>

#include <glm/glm.hpp>

#include <bklib/impl/win/win_platform.hpp>
#include <bklib/renderer2d.hpp>

namespace bklib {

class renderer2d::impl_t {
public:
    template <typename T>
    using com_ptr = bklib::detail::com_ptr<T>;

    impl_t(impl_t const&) = delete;
    impl_t& operator=(impl_t const&) = delete;
    ~impl_t() = default;

    impl_t(platform_window_handle handle);

    void resize(unsigned width, unsigned height) {
        target_->Resize(D2D1::SizeU(width, height));
    }

    void set_transform(glm::mat3 const& mat) {

        auto m = D2D1::Matrix3x2F(
            mat[0][0], mat[1][0],
            mat[0][1], mat[1][1],
            mat[2][0], mat[2][1]
        );

        target_->SetTransform(m);
    }

    void begin() {
        target_->BeginDraw();
        target_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    }

    void end() {
        HRESULT const hr = target_->EndDraw();
        if (FAILED(hr)) {
            //BK_DEBUG_BREAK();
        }
    }

    void clear() {
        color_alpha const clear_color {1.0f, 0.0f, 0.0f, 1.0f};
        clear(clear_color);
    }

    void clear(color const c) {
        color_alpha const clear_color {gfx::red(c), gfx::green(c), gfx::blue(c), 1.0f};
        clear(clear_color);
    }

    void clear(color_alpha const c) {
        auto const clear_color = D2D1::ColorF(
            gfx::red(c)
          , gfx::green(c)
          , gfx::blue(c)
          , gfx::alpha(c)
        );

        target_->Clear(clear_color);
    }

    void draw_text(bklib::string_ref  text);
    void draw_text(bklib::wstring_ref text);

    void draw_rect(rect const r, coord_t const width = 1.0f) {
        auto const half = width / 2.0f;
        auto const r1 = D2D1::RectF(
              r.left()   + half
            , r.top()    + half
            , r.right()  - half
            , r.bottom() - half
        );

        target_->DrawRectangle(r1, brush_.get(), width);
    }


    void draw_round_rect();

    void set_color_brush(color_alpha c) {
        auto const c1 = D2D1::ColorF(
            gfx::red(c)
          , gfx::green(c)
          , gfx::blue(c)
          , gfx::alpha(c)
        );

        brush_->SetColor(c1);
    }

    void fill_rect(rect const r) {
        auto const r1 = D2D1::RectF(
              r.left()
            , r.top()
            , r.right()
            , r.bottom()
        );
        target_->FillRectangle(r1, brush_.get());
    }
    void fill_round_rect();

    template <typename T>
    void draw_rect(bklib::axis_aligned_rect<T> const r, float width = 1.0f) {
        auto const half = width / 2.0f;

        draw_rect(
            D2D1::RectF(
                static_cast<float>(r.left())   + half
              , static_cast<float>(r.top())    + half
              , static_cast<float>(r.right())  - half
              , static_cast<float>(r.bottom()) - half
            )
          , width
        );
    }

    void draw_rect(float top, float left, float w, float h, float width = 1.0f) {
        draw_rect(
            D2D1::RectF(left, top, left + w, top + h)
          , width
        );
    }

    void draw_rect(D2D1_RECT_F const rect, float const width = 1.0f) {
        target_->DrawRectangle(rect, brush_.get(), width);
    }

    template <typename T>
    void draw_filled_rect(bklib::axis_aligned_rect<T> const r) {
        auto const rect = D2D1::RectF(r.left(), r.top(), r.right(), r.bottom());
        target_->FillRectangle(rect, brush_.get());
    }

    void draw_filled_rect(float top, float left, float w, float h) {
        target_->FillRectangle(D2D1::RectF(left, top, left + w, top + h), brush_.get());
    }

    com_ptr<ID2D1Bitmap> load_image();

    using rect = bklib::axis_aligned_rect<float>;

    static D2D_RECT_F& convert_rect(rect& r) {
        return *reinterpret_cast<D2D_RECT_F*>(&r);
    }

    void draw_image(ID2D1Bitmap& image, rect dest, rect src) {
        target_->DrawBitmap(
            &image
          , convert_rect(dest)
          , 1.0f
          , D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
          , convert_rect(src)
        );
    }

    void set_color(float r, float g, float b, float a = 1.0f) {
        brush_->SetColor(D2D1::ColorF(r, g, b, a));
    }
private:
    com_ptr<IWICImagingFactory>    wic_factory_;
    com_ptr<ID2D1Factory>          factory_;

    com_ptr<ID2D1HwndRenderTarget> target_;
    com_ptr<ID2D1SolidColorBrush>  brush_;
};

} //namespace bklib
