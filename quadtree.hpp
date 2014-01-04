//#include <bklib/math.hpp>
//
//template <typename T> struct quad_tree_node;
//template <typename T> class quad_tree;
//
//struct quad_rect {
//    using scalar_t = int;
//    using rect_t  = bklib::axis_aligned_rect<scalar_t>;
//    using point_t = bklib::point2d<scalar_t>;
//
//    static rect_t top_left(scalar_t const x, scalar_t const y, scalar_t const w, scalar_t const h) {
//        return {x, y, x + w, y + h};
//    }
//
//    static rect_t top_right(scalar_t const x, scalar_t const y, scalar_t const w, scalar_t const h) {
//        return {x + w, y, x + 2*w, y + h};
//    }
//
//    static rect_t bottom_left(scalar_t const x, scalar_t const y, scalar_t const w, scalar_t const h) {
//        return {x, y + h, x + w, y + 2*h};
//    }
//
//    static rect_t bottom_right(scalar_t const x, scalar_t const y, scalar_t const w, scalar_t const h) {
//        return {x + w, y + h, x + 2*w, y + 2*h};
//    }
//
//    quad_rect(scalar_t const x, scalar_t const y, scalar_t const w, scalar_t const h)
//      : quadrant {{
//            top_left(x, y, w, h)
//          , top_right(x, y, w, h)
//          , bottom_left(x, y, w, h)
//          , bottom_right(x, y, w, h)
//        }}
//    {
//    }
//
//    quad_rect(rect_t r)
//      : quad_rect {r.left(), r.top(), r.width() / 2, r.height() / 2}
//    {
//    }
//
//    std::array<bool, 4> intersect(rect_t const r) const {
//        std::array<int, 4> indicies;
//
//        indicies[0] = intersect(r.top_left());
//        indicies[1] = intersect(r.top_right());
//        indicies[2] = intersect(r.bottom_left());
//        indicies[3] = intersect(r.bottom_right());
//
//        std::array<bool, 4> result {{false}};
//
//        for (auto const i : indicies) {
//            if (i >= 0) result[i] = true;
//        }
//
//        return result;
//    }
//
//    int intersect(point_t const p) const {
//        for (size_t i = 0; i < 4; ++i) {
//            if (bklib::intersects(p, quadrant[i])) return i;
//        }
//
//        return -1;
//    }
//
//    rect_t const& operator[](size_t i) const {
//        return quadrant[i];
//    }
//
//    std::array<rect_t, 4> quadrant;
//};
//
//template <typename T>
//struct quad_tree_traits {
//    using pointer = T*;
//    using rect    = bklib::axis_aligned_rect<int>;
//    using point   = bklib::point2d<int>;
//};
//
//template <typename T>
//struct quad_tree_node : public quad_tree_traits<T> {
//    static size_t const MAX_SIZE = 32;
//
//    using tree_t = std::array<std::unique_ptr<quad_tree_node>, 4>;
//    using leaf_t = std::vector<pointer>;
//    using data_t = boost::variant<leaf_t, tree_t>;
//
//    bool is_leaf() const {
//        struct visitor : public boost::static_visitor<bool> {
//            bool operator()(tree_t const&) const { return false; }
//            bool operator()(leaf_t const&) const { return true; }
//        };
//
//        return boost::apply_visitor(visitor{}, data);
//    }
//
//    bool is_full() const {
//        struct visitor : public boost::static_visitor<bool> {
//            bool operator()(tree_t const&) const { return false; }
//            bool operator()(leaf_t const& leaf) const { return leaf.size() > MAX_SIZE; }
//        };
//
//        return boost::apply_visitor(visitor{}, data);
//    }
//
//    leaf_t split() {
//        BK_ASSERT(is_leaf());
//
//        leaf_t result = std::move(boost::get<leaf_t>(data));
//
//        data = tree_t {{
//            std::make_unique<quad_tree_node>()
//          , std::make_unique<quad_tree_node>()
//          , std::make_unique<quad_tree_node>()
//          , std::make_unique<quad_tree_node>()
//        }};
//
//        return result;
//    }
//
//    void insert(pointer const value) {
//        BK_ASSERT(is_leaf());
//        boost::get<leaf_t>(data).push_back(value);
//    }
//
//    std::array<bool, 4> which(rect const& bounds, point const p) const {
//        auto const i = quad_rect{bounds}.intersect(p);
//        BK_ASSERT(i >= 0);
//
//        std::array<bool, 4> result = {{false}};
//        result[static_cast<size_t>(i)] = true;
//
//        return result;
//    }
//
//    std::array<bool, 4> which(rect const& bounds, rect const& r) const {
//        return quad_rect{bounds}.intersect(r);
//    }
//
//    quad_tree_node* child(size_t const i) {
//        BK_ASSERT(i < 4);
//        BK_ASSERT(!is_leaf());
//        return boost::get<tree_t>(data)[i].get();
//    }
//
//    data_t data;
//};
//
//template <typename T>
//class quad_tree : public quad_tree_traits<T> {
//public:
//    using node_t = quad_tree_node<T>;
//
//    quad_tree() : bounds_ {0, 0, 2, 2} {}
//
//    void insert(pointer const value) {
//        auto const bounds = get_bounds(*value);
//
//        adjust_bounds_(bounds);
//
//        buffer_.clear();
//        buffer_.push_back(&root_);
//
//        node_t* node = nullptr;
//
//        for (size_t i = 0; i < buffer_.size(); ++i) {
//            node = buffer_[i];
//
//            if (node->is_leaf()) {
//                node->insert(value);
//                if (node->is_full()) {
//                    split_(node);
//                }
//                continue;
//            }
//
//            auto const quads = node->which(bounds_, bounds);
//            for (size_t j = 0; j < 4; ++j) {
//                if (quads[j]) {
//                    buffer_.push_back(node->child(j));
//                }
//            }
//        }
//
//    }
//private:
//    void split_(node_t* node) {
//        for (auto value : node->split()) {
//            auto const bounds = get_bounds(*value);
//            auto const quads  = node->which(bounds_, bounds);
//            for (size_t i = 0; i < 4; ++i) {
//                if (quads[i]) {
//                    node->child(i)->insert(value);
//                }
//            }
//        }
//    }
//
//    void adjust_bounds_(rect const r) {
//        if (bklib::intersection_of(r, bounds_).result == r) {
//            return;
//        }
//
//        auto x0 = std::min(r.left(),   bounds_.left())   + 1;
//        auto y0 = std::min(r.top(),    bounds_.top())    + 1;
//        auto x1 = std::max(r.right(),  bounds_.right())  + 1;
//        auto y1 = std::max(r.bottom(), bounds_.bottom()) + 1;
//
//        x0 += x0 % 2;
//        y0 += y0 % 2;
//        x1 += x1 % 2;
//        y1 += y1 % 2;
//
//        bounds_ = rect {x0, y0, x1, y1};
//    }
//
//    void adjust_bounds_(point const p) {
//        if (bklib::intersects(p, bounds_)) {
//            return;
//        }
//
//        auto px = std::abs(p.x) + 1; px += px % 2;
//        auto py = std::abs(p.y) + 1; py += py % 2;
//
//        auto const x = std::max(bounds_.right(),  px);
//        auto const y = std::max(bounds_.bottom(), py);
//
//        bounds_ = rect {-x, -y, x, y};
//
//        BK_ASSERT(bklib::intersects(p, bounds_));
//    }
//
//    rect                 bounds_;
//    node_t               root_;
//    std::vector<node_t*> buffer_;
//};
//
//bklib::axis_aligned_rect<int> const& get_bounds(bklib::axis_aligned_rect<int> const& bounds) {
//    return bounds;
//}
