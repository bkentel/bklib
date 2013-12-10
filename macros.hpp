
#define BK_CAT2_IMPL(A1, A2) A1 ## A2
#define BK_CAT2(A1, A2) BK_CAT2_IMPL(A1, A2)

#if defined(BOOST_COMP_MSVC)
#   define BK_UNIQUE_ID(NAME) BK_CAT2(NAME, __COUNTER__)
#else
#   define BK_UNIQUE_ID(NAME) BK_CAT2(NAME, __LINE__)
#endif

#define BK_UNUSED(x) (void)(x)

#define BK_NO_COPY_ASSIGN(name)\
name(name const&) = delete;\
name& operator=(name const&) = delete

#define BK_DELETE_ALL(name)\
name(name const&) = delete;\
name(name&&) = delete;\
name& operator=(name const&) = delete;\
name& operator=(name&&) = delete

#define BK_DEFAULT_ALL(name)\
name(name const&) = default;\
name(name&&) = default;\
name& operator=(name const&) = default;\
name& operator=(name&&) = default
