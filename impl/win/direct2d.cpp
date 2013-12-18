#include "direct2d.hpp"

using bklib::platform_window_handle;
using bklib::detail::make_com_error;
using bklib::detail::make_com_ptr;
using bklib::detail::com_ptr;

using impl_t = bklib::renderer2d::impl_t;

namespace {
//------------------------------------------------------------------------------
auto create_wic_factory() {
    IWICImagingFactory* factory = nullptr;

    HRESULT const hr = ::CoCreateInstance(
        CLSID_WICImagingFactory
      , nullptr
      , CLSCTX_INPROC_SERVER
      , IID_PPV_ARGS(&factory)
    );

    if (FAILED(hr)) {
        make_com_error("CoCreateInstance(IWICImagingFactory)", hr);
    }

    return com_ptr<IWICImagingFactory>(factory);
}
//------------------------------------------------------------------------------
auto create_factory() {
    ID2D1Factory* factory = nullptr;

    HRESULT const hr = ::D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED
      , &factory
    );

    if (FAILED(hr)) {
        make_com_error("D2D1CreateFactory", hr);
    }

    return com_ptr<ID2D1Factory>(factory);
}
//------------------------------------------------------------------------------
auto create_renderer(ID2D1Factory& factory, HWND window) {
    // Obtain the size of the drawing area.
    RECT window_rect {0};
    ::GetClientRect(window, &window_rect);

    // Create a Direct2D render target			
    ID2D1HwndRenderTarget* target = nullptr;
		
    HRESULT const hr = factory.CreateHwndRenderTarget(
        D2D1::RenderTargetProperties()
      , D2D1::HwndRenderTargetProperties(
            window
          , D2D1::SizeU(
                window_rect.right  - window_rect.left
              , window_rect.bottom - window_rect.top
            )
        )
      , &target
    );
        
    if (FAILED(hr)) {
        make_com_error("ID2D1Factory::CreateHwndRenderTarget", hr);
    }

    return com_ptr<ID2D1HwndRenderTarget>(target);
}
//------------------------------------------------------------------------------
auto create_brush(ID2D1HwndRenderTarget& target) {
    ID2D1SolidColorBrush* brush = nullptr;

    HRESULT const hr = target.CreateSolidColorBrush(
        D2D1::ColorF(1.0f, 1.0f, 1.0f)
      , &brush
    );

    if (FAILED(hr)) {
        make_com_error("ID2D1HwndRenderTarget::CreateSolidColorBrush", hr);
    }

    return com_ptr<ID2D1SolidColorBrush>(brush);
}
//------------------------------------------------------------------------------
auto create_decoder_from_file(
    IWICImagingFactory& factory
  , std::wstring const& file_name
) {
    IWICBitmapDecoder* decoder = nullptr;

    auto const hr = factory.CreateDecoderFromFilename(
        file_name.data(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );

    if (FAILED(hr)) {
        make_com_error("IWICImagingFactory::CreateDecoderFromFilename", hr);
    }

    return com_ptr<IWICBitmapDecoder>(decoder);
}
//------------------------------------------------------------------------------
}

impl_t::impl_t(platform_window_handle handle)
  : x_off_{0.0f}, y_off_{0.0f}
  , x_scale_{1.0f}, y_scale_{1.0f}
  , wic_factory_(create_wic_factory())
  , factory_(create_factory())
  , target_(create_renderer(*factory_, handle))
  , brush_(create_brush(*target_))
{
}

com_ptr<ID2D1Bitmap> impl_t::load_image() {
    wchar_t const file_name[] = L"./data/tiles.png";

    auto decoder = create_decoder_from_file(*wic_factory_, file_name);

    auto source = [&] {
        IWICBitmapFrameDecode* source = nullptr;
        auto const hr = decoder->GetFrame(0, &source);
        BK_COM_THROW_IF_FAILED("IWICBitmapDecoder::GetFrame", hr);
        return make_com_ptr(source);
    }();

    auto converter = [&] {
        IWICFormatConverter* converter = nullptr;
        auto const hr = wic_factory_->CreateFormatConverter(&converter);
        BK_COM_THROW_IF_FAILED("IWICImagingFactory::CreateFormatConverter", hr);
        return make_com_ptr(converter);
    }();

    auto hr = converter->Initialize(
        source.get()
      , GUID_WICPixelFormat32bppPBGRA
      , WICBitmapDitherTypeNone
      , nullptr
      , 0.0f
      , WICBitmapPaletteTypeMedianCut
    );
    BK_COM_THROW_IF_FAILED("IWICFormatConverter::Initialize", hr);

    ID2D1Bitmap* bitmap = nullptr;
    hr = target_->CreateBitmapFromWicBitmap(
        converter.get()
      , nullptr
      , &bitmap
    );
    BK_COM_THROW_IF_FAILED("ID2D1HwndRenderTarget::CreateBitmapFromWicBitmap", hr);

    return make_com_ptr(bitmap);
}
