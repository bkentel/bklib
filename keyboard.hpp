#pragma once

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include "types.hpp"
#include "callback.hpp"
#include "util.hpp"
#include "macros.hpp"

namespace bklib {
//==============================================================================
enum class keycode : uint8_t;

class key_combo;
class keyboard;
//==============================================================================
BK_DECLARE_EVENT(on_keydown,   void (keyboard& state, keycode key));
BK_DECLARE_EVENT(on_keyup,     void (keyboard& state, keycode key));
BK_DECLARE_EVENT(on_keyrepeat, void (keyboard& state, keycode key));
//==============================================================================
//! Keyboard keys; roughly aligned with ASCII codes.
//==============================================================================
enum class keycode : uint8_t {
    NONE
  , SPACE = ' '
  , K0 = '0', K1, K2, K3, K4, K5, K6, K7, K8, K9
  , A  = 'A', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
  , NUM_0, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9
  , NUM_DIV, NUM_MUL, NUM_MIN, NUM_ADD, NUM_DEC, NUM_ENTER, NUM_LCK
  , F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17
  , F18, F19, F20, F21, F22, F23, F24
  , LEFT, RIGHT, UP, DOWN
  , CTRL_L, CTRL_R
  , ALT_L, ALT_R
  , SHIFT_L, SHIFT_R
  , ENTER
  , INS, HOME, PAGE_UP, DEL, END, PAGE_DOWN,
};

//==============================================================================
//! Bundled set of keyboard keys.
//==============================================================================
class key_combo {
public:
    BK_DEFAULT_ALL(key_combo);

    key_combo() = default;

    key_combo(std::initializer_list<keycode> const list)
      : keys_ {list.begin(), list.end()}
    {
    }

    void add(keycode const key) {
        keys_.insert(key);
    }

    template <typename It>
    void add(It first, It last) {
        keys_.insert(first, last);
    }

    void remove(keycode const key) {
        keys_.erase(key);
    }

    void clear() {
        keys_.clear();
    }

    bool includes(key_combo const& other) const {
        return std::includes(
            std::cbegin(keys_), std::cend(keys_)
          , std::cbegin(other.keys_), std::cend(other.keys_)
        );
    }

    bool includes(keycode const key) const {
        return keys_.cend() != keys_.find(key);
    }

    size_t size()  const { return keys_.size(); }
    bool   empty() const { return keys_.empty(); }

    auto begin() { return keys_.begin(); }
    auto end()   { return keys_.end(); }

    auto begin()  const { return keys_.begin(); }
    auto end()    const { return keys_.end(); }

    auto cbegin() const { return keys_.cbegin(); }
    auto cend()   const { return keys_.cend(); }
private:
    boost::container::flat_set<keycode> keys_;
};

inline bool operator==(key_combo const& lhs, key_combo const& rhs) {
    return std::equal(std::cbegin(lhs), std::cend(lhs), std::cbegin(rhs));
}

inline bool operator<(key_combo const& lhs, key_combo const& rhs) {
    return std::lexicographical_compare(
        std::cbegin(lhs), std::cend(lhs)
      , std::cbegin(rhs), std::cend(rhs)
    );
}

inline bool operator!=(key_combo const& lhs, key_combo const& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, key_combo const& combo);
//==============================================================================

//==============================================================================
//! The state of the keyboard.
//==============================================================================
class keyboard {
public:
    //--------------------------------------------------------------------------
    struct record {
        time_point time;    //!< Time stamp of when the keystate changed.
        bool       is_down; //!< The key is down.

        explicit operator bool() const BK_NOEXCEPT { return is_down; }
    };
    //--------------------------------------------------------------------------
    static size_t const KEY_COUNT = 0xFF;
public:
    static string_ref translate(keycode key);
    static keycode    translate(utf8string name);
    static keycode    translate(string_ref name);
    static keycode    translate(hash_t hash);
public:
    BK_NO_COPY_ASSIGN(keyboard);

    keyboard();

    record operator[](keycode const k) const {
        return state_[get_enum_value(k)];
    }

    key_combo const& state() const { return keys_; }

    bool set_down(keycode key);
    bool set_up(keycode key);

    void clear();
private:
    key_combo                     keys_;  //!< Current set of down keys.
    std::array<record, KEY_COUNT> state_; //!< State of each key.
};



} //namespace bklib
