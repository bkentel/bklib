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
key_combo::key_combo() {
    keys_.reserve(4);
}
//==============================================================================
key_combo::key_combo(std::initializer_list<keycode> const list)
    : keys_{list.begin(), list.end()}
{
    std::sort(std::begin(keys_), std::end(keys_), pred{});
}    
//==============================================================================
bool key_combo::add(keycode const key) {
    auto const result = keys_.insert(key);
    return result.second;
}
//==============================================================================
bool key_combo::add(std::initializer_list<keycode> const list) {
    auto const size_before = keys_.size();
    auto const size_after  = size_before + list.size();

    keys_.reserve(size_after);
    keys_.insert(list.begin(), list.end());
    
    return size_after == keys_.size();    
}
//==============================================================================
bool key_combo::remove(keycode const key) {
    auto const it = keys_.find(key);
    if (it == std::cend(keys_)) {
        return false;
    }
    
    keys_.erase(it);

    return true;
}
//==============================================================================
std::ostream& bklib::operator<<(std::ostream& out, key_combo const& combo) {
    out << "[";
    for (auto k : combo) {
        out << keyboard::key_name(k) << ", ";
    }
    out << "]";

    return out;
}

bool bklib::operator==(key_combo const& lhs, key_combo const& rhs) {
    return std::equal(
        std::cbegin(lhs)
      , std::cend(lhs)
      , std::cbegin(rhs)
    );
}
//==============================================================================
bool bklib::operator<(key_combo const& lhs, key_combo const& rhs) {
    return std::lexicographical_compare(
        std::cbegin(lhs), std::cend(lhs)
      , std::cbegin(rhs), std::cend(rhs)
    );
}

////////////////////////////////////////////////////////////////////////////////
// bklib::keyboard
////////////////////////////////////////////////////////////////////////////////
namespace {
namespace local_state {
    static boost::container::flat_map<hash_t, keycode>     hash_key_map;    
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
string_ref keyboard::key_name(keycode const key) {
    local_state::init();
    return bklib::find_or(local_state::key_string_map, key, string_ref{});
}
//==============================================================================
keycode keyboard::key_code(utf8string const& name) {
    return key_code(utf8string_hash(name));
}
//==============================================================================
keycode keyboard::key_code(string_ref const name) {
    return key_code(utf8string_hash(name));
}
//==============================================================================
keycode keyboard::key_code(hash_t const hash) {
    local_state::init();
    return bklib::find_or(local_state::hash_key_map, hash, keycode::NONE);
}
//==============================================================================
keyboard::keyboard() {
    record const r = {clock::now(), false};
    std::fill(std::begin(state_), std::end(state_), r);
}
//==============================================================================  
bool keyboard::set_state(keycode const key, bool const is_down) {
    auto const i = bklib::get_enum_value(key);

    if (state_[i].is_down == is_down) return true;

    state_[i].is_down = is_down;
    state_[i].time    = clock::now();

    auto const result = is_down
      ? keys_.add(key)
      : keys_.remove(key);

    BK_ASSERT(result);

    return false;
}
//==============================================================================
void keyboard::clear(bklib::on_keyup const& functor) {
    keys_.clear();

    auto const now = clock::now();

    size_t code = 0;
    for (auto& r : state_) {
        if (r.is_down) {
            r.is_down = false;
            r.time    = now;

            if (functor) {
                functor(*this, static_cast<keycode>(code));
            }
        }
        code++;
    }
}
//==============================================================================
