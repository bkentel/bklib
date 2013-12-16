#pragma once

#include <type_traits>
#include <memory>

#include <bklib/impl/win/win_platform.hpp>
#include <bklib/math.hpp>

namespace bklib {
namespace detail {

class d2d_renderer {
public:
    d2d_renderer(d2d_renderer const&) = delete;
    d2d_renderer& operator=(d2d_renderer const&) = delete;
    ~d2d_renderer() = default;

    d2d_renderer(HWND window);

    void resize(unsigned w, unsigned h) {
        target_->Resize(D2D1::SizeU(w, h));
    }

    void begin() {
        target_->BeginDraw();

        auto mat = D2D1::Matrix3x2F(
            x_scale_, 0.0f
          , 0.0f,     y_scale_
          , x_off_,   y_off_
        );

        target_->SetTransform(mat);

        target_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    }

    void end() {
        HRESULT const hr = target_->EndDraw();
        if (FAILED(hr)) {
            BK_DEBUG_BREAK();
        }
    }

    void clear() {
        target_->Clear(D2D1::ColorF(1.0, 0.0, 0.0));
    }

    void translate(float dx, float dy) {
        x_off_ += dx;
        y_off_ += dy;
    }

    void scale(float s) {
        x_scale_ = s;
        y_scale_ = s;
    }

    void skew(float sx, float sy) {
        x_scale_ = sx;
        y_scale_ = sy;
    }

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
    float x_off_;
    float y_off_;
    float x_scale_;
    float y_scale_;

    com_ptr<IWICImagingFactory>    wic_factory_;
    com_ptr<ID2D1Factory>          factory_;

    com_ptr<ID2D1HwndRenderTarget> target_;
    com_ptr<ID2D1SolidColorBrush>  brush_;
};

} //namespace detail
} //namespace bklib
