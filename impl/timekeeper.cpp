#include "timekeeper.hpp"
#include "assert.hpp"

using bklib::timekeeper;

//==============================================================================
//! 
//==============================================================================
timekeeper::timekeeper() {
}
//==============================================================================
//! 
//==============================================================================
timekeeper::handle timekeeper::register_event_(duration period, callback f) {
    using namespace std::placeholders;

    BK_ASSERT(f);
    BK_ASSERT(period.count() > 0);

    auto const now = clock::now();
    auto const deadline = now + period;

    auto const index = records_.size();
    timekeeper::handle const handle = { index };

    records_.emplace_back(record {
        handle, std::move(f), period, deadline            
    });

    heap_.emplace_back(index);
    std::push_heap(
        std::begin(heap_)
      , std::end(heap_)
      , std::bind(&timekeeper::heap_predidate_, this, _1, _2)
    );

    return handle;
}
//==============================================================================
//! 
//==============================================================================
void timekeeper::update() {
    using namespace std::placeholders;

    auto const now = clock::now();
    duration   dt  = std::chrono::seconds(1);

    while (!heap_.empty() && dt.count() >= 0) {
        std::pop_heap(
            std::begin(heap_)
          , std::end(heap_)
          , std::bind(&timekeeper::heap_predidate_, this, _1, _2)
        );

        auto& rec = records_[heap_.back()];
        dt = now - rec.deadline;

        if (dt.count() >= 0) {
            rec.callback(std::chrono::duration_cast<delta>(dt + rec.period));
            rec.deadline = now + rec.period;
        }

        std::push_heap(
            std::begin(heap_)
          , std::end(heap_)
          , std::bind(&timekeeper::heap_predidate_, this, _1, _2)
        );
    }
}
