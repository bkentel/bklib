#include "keyboard.hpp"
#include "assert.hpp"

using bklib::keyboard;
using bklib::key_combo;
using bklib::string_ref;
using bklib::hash_t;
using bklib::keycode;
using bklib::utf8string_hash;
using bklib::utf8string;

////////////////////////////////////////////////////////////////////////////////
// bklib::key_combo
////////////////////////////////////////////////////////////////////////////////
struct separator {
    separator() = default;
    separator(bklib::utf8string delim) : delim{std::move(delim)} {}

    bklib::utf8string delim = ", ";
    size_t            count = 0;
};

std::ostream& operator<<(std::ostream& out, separator& s) {
    if (s.count++ == 0) {
        return out;
    }

    return (out << s.delim);
}

std::ostream& bklib::operator<<(std::ostream& out, key_combo const& combo) {
    separator sep;

    out << "[";

    for (auto k : combo) {
        out << sep << keyboard::translate(k);
    }

    out << "]";

    return out;
}

////////////////////////////////////////////////////////////////////////////////
// bklib::keyboard
////////////////////////////////////////////////////////////////////////////////
namespace {
namespace local_state {
    static boost::container::flat_map<hash_t,  keycode>    hash_key_map;
    static boost::container::flat_map<keycode, string_ref> key_string_map;

    static std::once_flag once_flag;

    #define BK_ADD_KEY_HASH(ENUM)\
    [&] {\
        hash_t  const hash     {utf8string_hash(#ENUM)};\
        keycode const code     {ENUM};\
        \
        auto const result0 = hash_key_map.emplace(std::make_pair(hash, code));\
        BK_ASSERT(result0.second && "collision");\
        \
        auto const result1 = key_string_map.emplace(std::make_pair(code, #ENUM));\
        BK_ASSERT(result1.second && "duplicate");\
    }()

    void init_maps() {
        using KEY = keycode;
        BK_ADD_KEY_HASH(KEY::NUM_0); BK_ADD_KEY_HASH(KEY::NUM_1);
        BK_ADD_KEY_HASH(KEY::NUM_2); BK_ADD_KEY_HASH(KEY::NUM_3);
        BK_ADD_KEY_HASH(KEY::NUM_4); BK_ADD_KEY_HASH(KEY::NUM_5);
        BK_ADD_KEY_HASH(KEY::NUM_6); BK_ADD_KEY_HASH(KEY::NUM_7);
        BK_ADD_KEY_HASH(KEY::NUM_8); BK_ADD_KEY_HASH(KEY::NUM_9);

        BK_ADD_KEY_HASH(KEY::LEFT); BK_ADD_KEY_HASH(KEY::RIGHT);
        BK_ADD_KEY_HASH(KEY::UP);   BK_ADD_KEY_HASH(KEY::DOWN);

        BK_ADD_KEY_HASH(KEY::SPACE);
    }

    #undef BK_ADD_KEY_HASH

    void init() {
        std::call_once(once_flag, init_maps);
    }
} //namespace local_state
} //namespace
//==============================================================================
string_ref keyboard::translate(keycode const key) {
    local_state::init();
    return bklib::find_or(local_state::key_string_map, key, string_ref{});
}
//==============================================================================
keycode keyboard::translate(utf8string const name) {
    return translate(utf8string_hash(name));
}
//==============================================================================
keycode keyboard::translate(string_ref const name) {
    return translate(utf8string_hash(name));
}
//==============================================================================
keycode keyboard::translate(hash_t const hash) {
    local_state::init();
    return bklib::find_or(local_state::hash_key_map, hash, keycode::NONE);
}
//==============================================================================
keyboard::keyboard() {
    record const r = {clock_t::now(), false};
    std::fill(std::begin(state_), std::end(state_), r);
}
//==============================================================================
bool keyboard::set_down(keycode const key) {
    auto const k = get_enum_value(key);
    record& rec = state_[k];

    if (rec.is_down) {
        return true;
    }

    rec.is_down = true;
    rec.time    = clock_t::now();

    keys_.add(key);

    return false;
}
//==============================================================================
bool keyboard::set_up(keycode key) {
    auto const k = get_enum_value(key);
    record& rec = state_[k];

    if (!rec.is_down) {
        return true;
    }

    rec.is_down = false;
    rec.time    = clock_t::now();

    keys_.remove(key);

    return false;
}
//==============================================================================
void keyboard::clear() {
    keys_.clear();

    record const rec {clock_t::now(), false};
    std::fill(std::begin(state_), std::end(state_), rec);
}
//==============================================================================
