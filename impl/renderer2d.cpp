#include <bklib/renderer2d.hpp>

#if defined(BOOST_OS_WINDOWS)
#   include <bklib/impl/win/direct2d.hpp>
#else
#   error "fill me in"
#endif

using bklib::renderer2d;

renderer2d::renderer2d(bklib::platform_window_handle const handle)
  : impl_{std::make_unique<impl_t>(handle)}
{
}

renderer2d::~renderer2d() {
}

void renderer2d::resize(unsigned width, unsigned height) {
    impl_->resize(width, height);
}

void renderer2d::begin() {
    impl_->begin();
}

void renderer2d::end() {
    impl_->end();
}

void renderer2d::clear(color_alpha c) {
    impl_->clear(c);
}

void renderer2d::draw_text(bklib::string_ref text) {
}

void renderer2d::draw_text(bklib::wstring_ref text) {
}

void renderer2d::draw_rect(rect const r, coord_t const width) {
    impl_->draw_rect(r, width);
}

void renderer2d::draw_round_rect() {
}

void renderer2d::fill_rect(rect const r) {
    impl_->fill_rect(r);
}

void renderer2d::fill_round_rect() {
}

void renderer2d::set_color_brush(color_alpha c) {
    impl_->set_color_brush(c);
}
