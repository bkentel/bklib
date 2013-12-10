#pragma once

#include <chrono>
#include <vector>
#include <functional>

#include "config.hpp"

namespace bklib {

class timekeeper {
public:
    using clock      = std::chrono::high_resolution_clock;
    using duration   = clock::duration;
    using time_point = clock::time_point;
    using delta      = std::chrono::milliseconds;
    using callback   = std::function<void (delta dt)>;
    
    struct handle { size_t id; };

    struct record {
        timekeeper::handle     handle;
        timekeeper::callback   callback;
        timekeeper::duration   period;
        timekeeper::time_point deadline;
    };

    timekeeper();

    //! Register a callback @c f to be called every @c period of time.
    template <typename T>
    handle register_event(T period, callback f) {
        return register_event_(
            std::chrono::duration_cast<duration>(period)
          , std::move(f)
        );
    }

    //! Update the time and execute all callbacks which have met or exceeded
    //! their deadlines.
    void update();
private:
    //! predicate to maintain a min heap.
    bool heap_predidate_(uint16_t a, uint16_t b) const BK_NOEXCEPT {
        return records_[a].deadline > records_[b].deadline;
    }

    handle register_event_(duration period, callback f);

    std::vector<record>   records_; //!< unsorted records
    std::vector<uint16_t> heap_;    //!< sorted (heap) of indicies into records_
};

} // namespace bklib
