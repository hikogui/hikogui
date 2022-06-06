// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grapheme.hpp"
#include "unicode_normalization.hpp"
#include "unicode_description.hpp"
#include <mutex>

namespace hi::inline v1 {
namespace detail {

class long_grapheme_deleter {
public:
    using allocator = std::allocator<std::byte>;

    constexpr long_grapheme_deleter() noexcept = default;
    constexpr long_grapheme_deleter(long_grapheme_deleter const&) noexcept = default;
    constexpr long_grapheme_deleter(long_grapheme_deleter&&) noexcept = default;
    constexpr long_grapheme_deleter& operator=(long_grapheme_deleter const&) noexcept = default;
    constexpr long_grapheme_deleter& operator=(long_grapheme_deleter&&) noexcept = default;

    void operator()(long_grapheme *lg) const noexcept
    {
        hilet num_bytes = sizeof(long_grapheme) + lg->size() * sizeof(char32_t);

        std::destroy(lg->begin(), lg->end());
        std::destroy_at(lg);

        static_assert(std::allocator_traits<allocator>::is_always_equal::value);
        auto alloc = allocator{};

        auto *ptr = reinterpret_cast<std::byte *>(lg);
        std::allocator_traits<allocator>::deallocate(alloc, ptr, num_bytes);
    }
};

using long_grapheme_unique_ptr = std::unique_ptr<long_grapheme, long_grapheme_deleter>;

static long_grapheme_unique_ptr make_long_grapheme(std::u32string_view code_points) noexcept
{
    // Allocate a buffer that holds a long_grapheme and the set of code-points.
    static_assert(std::allocator_traits<long_grapheme_deleter::allocator>::is_always_equal::value);
    auto alloc = long_grapheme_deleter::allocator{};

    hilet num_bytes = sizeof(long_grapheme) + code_points.size() * sizeof(char32_t);
    auto *ptr = std::allocator_traits<long_grapheme_deleter::allocator>::allocate(alloc, num_bytes);

    // Construct the long_grapheme and the code-points in the allocated buffer.
    auto *lg = std::construct_at<long_grapheme>(reinterpret_cast<long_grapheme *>(ptr), code_points.size());
    ptr += sizeof(long_grapheme);
    for (auto i = 0_uz; i != code_points.size(); ++i, ptr += sizeof(char32_t)) {
        std::construct_at<char32_t>(reinterpret_cast<char32_t *>(ptr, code_points[i]));
    }

    return {lg, long_grapheme_deleter{}};
}

class long_graphemes_type {
public:
    long_graphemes_type() noexcept = default;
    long_graphemes_type(long_graphemes_type const&) = delete;
    long_graphemes_type(long_graphemes_type&&) = delete;
    long_graphemes_type& operator=(long_graphemes_type const&) = delete;
    long_graphemes_type& operator=(long_graphemes_type&&) = delete;

    long_grapheme const *get(std::u32string_view code_points) const noexcept
    {
        hilet lock = std::scoped_lock(_mutex);

        auto it = std::lower_bound(_ordered_list.begin(), _ordered_list.end(), code_points, [](hilet& item, hilet& value) {
            return item->lexicographical_compare(value);
        });
        if (not(*it)->equal(code_points)) {
            it = _ordered_list.insert(it, make_long_grapheme(code_points));
        }

        return it->get();
    }

private:
    mutable unfair_mutex _mutex;
    mutable std::vector<long_grapheme_unique_ptr> _ordered_list;
};

static long_graphemes_type long_graphemes;

} // namespace detail

[[nodiscard]] static grapheme::value_type make_grapheme(std::u32string_view code_points) noexcept
{
    grapheme::value_type value = 0;

    switch (code_points.size()) {
    case 3:
        value = code_points[2];
        value <<= 21;
        [[fallthrough]];
    case 2:
        value |= code_points[1];
        value <<= 21;
        [[fallthrough]];
    case 1:
        value |= code_points[0];
        value <<= 1;
        value |= 1;
        [[fallthrough]];
    case 0:
        return value;
    default:
        return std::bit_cast<grapheme::value_type>(detail::long_graphemes.get(code_points));
    }
}

grapheme::grapheme(std::u32string_view code_points) noexcept : value(make_grapheme(unicode_NFKC(code_points))) {}

grapheme& grapheme::operator=(std::u32string_view code_points) noexcept
{
    value = make_grapheme(unicode_NFKC(code_points));
    return *this;
}

[[nodiscard]] grapheme grapheme::from_composed(std::u32string_view code_points) noexcept
{
    grapheme r;
    r.value = make_grapheme(code_points);
    return r;
}

[[nodiscard]] std::u32string grapheme::decomposed() const noexcept
{
    return unicode_NFD(composed());
}

[[nodiscard]] bool grapheme::valid() const noexcept
{
    if (empty()) {
        return false;
    }

    if (is_noncharacter(get<0>(*this))) {
        return false;
    }

    hilet& description = unicode_description::find(get<0>(*this));
    if (is_C(description)) {
        return false;
    }
    if (description.canonical_combining_class() != 0) {
        return false;
    }
    return true;
}

} // namespace hi::inline v1
