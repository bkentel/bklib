#pragma once

#include <type_traits>
#include <memory>

#include "win_platform.hpp"
#include "math.hpp"

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
    void draw_filled_rect(bklib::axis_aligned_rect<T> const r) {
        auto const rect = D2D1::RectF(r.left(), r.top(), r.right() - 1, r.bottom() - 1);
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
