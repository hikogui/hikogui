
#pragma once

#include "../memory.hpp"
#include "../type_traits.hpp"
#include <cstddef>
#include <memory>
#include <functional>
#include <string>
#include <string_view>

namespace hi::inline v1::detail {


class shared_state_path {
public:
    ~shared_state_path() noexcept = default;
    shared_state_path(shared_state_path const &) noexcept = default;
    shared_state_path(shared_state_path &&) noexcept = default;
    shared_state_path &operator=(shared_state_path const &) noexcept = default;
    shared_state_path &operator=(shared_state_path &&) noexcept = default;

    shared_state_path(std::string path, std::function<void *(void *)> converter) noexcept :
        _path(std::move(path)),
        _converter(std::move(converter)) {}

    shared_state_path() noexcept : shared_state_path({}, [](void *base_value) -> void * { return base_value; }) {}

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _path.empty();
    }

    [[nodiscard]] constexpr std::string const &path() const noexcept
    {
        return _path;
    }

    template<typename To>
    [[nodiscard]] To *convert(void *base_value) const noexcept
    {
        return static_cast<To *>(_converter(base_value));
    }

    template<typename From, basic_fixed_string Name>
    [[nodiscard]] shared_state_path by_name() const noexcept
    {
        return {
            std::format("{}.{}", _path, Name),
            [_converter](void *base_value) -> void * {
                return std::addressof(selector<From>{}.get<Name>(*static_cast<From *>(_converter(base_value))));
            }
        };
    }

    template<typename From, typename Index>
    [[nodiscard]] shared_state_path by_index(Index const &index) const noexcept
    {
        return {
            std::format("{}[{}]", _path, index),
            [_converter, index](void *base_value) -> void * {
                return std::addressof((*static_cast<From *>(_converter(base_value))[index]);
            }
        };
    }

    template<typename From, size_t Index>
    [[nodiscard]] shared_state_path by_index() const noexcept
    {
        return {
            std::format("{}[{}]", _path, Index),
            [_converter](void *base_value) -> void * {
                return std::addressof((*static_cast<From *>(_converter(base_value))[Index]);
            }
        };
    }

    [[nodiscard]] constexpr friend bool operator==(shared_state_path const &lhs, shared_state_path const &rhs) noexcept
    {
        return lhs._path == rhs._path;
    }

    [[nodiscard]] constexpr friend auto operator<=>(shared_state_path const &lhs, shared_state_path const &rhs) noexcept
    {
        return lhs._path <=> rhs._path;
    }

private:
    std::string _path;
    std::function<void *(void *)> _converter;
};

} // namespace hi::inline v1::detail
