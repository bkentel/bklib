#pragma once

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
//! Keyboard keys; roughly aligned with ASCII codes.
//==============================================================================
enum class keycode : uint8_t {
    NONE
  , SPACE = ' '
  , K0 = '0', K1, K2, K3, K4, K5, K6, K7, K8, K9
  , A  = 'A', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
  , NUM_0, NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9
  , NUM_DIV, NUM_MUL, NUM_MIN, NUM_ADD, NUM_DEC, NUM_ENTER, NUM_LCK
  , F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24
  , LEFT, RIGHT, UP, DOWN
  , CTRL_L, CTRL_R
  , ALT_L, ALT_R
  , SHIFT_L, SHIFT_R
  , ENTER
  , INS, HOME, PAGE_UP, DEL, END, PAGE_DOWN,
};

//==============================================================================
//! Key combos.
//==============================================================================
class key_combo {
public:
    using pred = std::greater<>;

    BK_DEFAULT_ALL(key_combo);

    key_combo();

    key_combo(std::initializer_list<keycode> list);

    bool add(keycode key);
    bool add(std::initializer_list<keycode> list);

    bool remove(keycode key);

    size_t size() const { return keys_.size(); }
    bool empty() const { return keys_.empty(); }

    void clear() { keys_.clear(); }

    auto begin() { return keys_.begin(); }
    auto end()   { return keys_.end(); }

    auto begin()  const { return keys_.begin(); }
    auto end()    const { return keys_.end(); }

    auto cbegin() const { return keys_.cbegin(); }
    auto cend()   const { return keys_.cend(); }
private:
    boost::container::flat_set<keycode, std::greater<>> keys_;
};

std::ostream& operator<<(std::ostream& out, key_combo const& combo);

bool operator==(key_combo const& lhs, key_combo const& rhs);
bool operator<(key_combo const& lhs, key_combo const& rhs);

inline bool operator!=(key_combo const& lhs, key_combo const& rhs) {
    return !(lhs == rhs);
}

inline bool operator>=(key_combo const& lhs, key_combo const& rhs) {
    return !(lhs < rhs);
}

BK_DECLARE_EVENT(on_keydown,   void (keyboard& state, keycode key));
BK_DECLARE_EVENT(on_keyup,     void (keyboard& state, keycode key));
BK_DECLARE_EVENT(on_keyrepeat, void (keyboard& state, keycode key));
//==============================================================================
//! The state of the keyboard.
//==============================================================================
class keyboard {
public:
    using clock      = std::chrono::high_resolution_clock;
    using time_point = clock::time_point;
    //--------------------------------------------------------------------------
    struct record {
        time_point time;    //!< Time stamp of when the keystate changed.
        bool       is_down; //!< The key is down.

        explicit operator bool() const BK_NOEXCEPT { return is_down; }
    };
    //--------------------------------------------------------------------------
    static BK_CONSTEXPR size_t const KEY_COUNT = 0xFF;

    static string_ref key_name(keycode key);
    static keycode    key_code(utf8string const& name);
    static keycode    key_code(string_ref name);
    static keycode    key_code(hash_t hash);

    keyboard(keyboard const&) = delete;
    keyboard& operator=(keyboard const&) = delete;

    keyboard();

    record operator[](keycode const k) const {
        return state_[get_enum_value(k)];
    }
    
    bool set_state(keycode key, bool is_down);
    void clear(on_keyup const& functor);
private:
    key_combo                     keys_;  //!< Current set of down keys.
    std::array<record, KEY_COUNT> state_; //!< State of each key.
};

} //namespace bklib
