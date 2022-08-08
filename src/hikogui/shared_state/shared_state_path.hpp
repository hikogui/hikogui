
#pragma once

#include "../memory.hpp"
#include <cstddef>
#include <memory>
#include <functional>
#include <string>
#include <string_view>

namespace hi::inline v1 {

struct member_offset {
    ptrdiff_t value;
};
#define hi_member_offset(x, name) member_offset<decltype(x)>{offsetof(x, name)};

class shared_state_path {
public:
    shared_state_path(
        std::shared_ptr<shared_state_path> parent,
        std::string_view name,
        std::function<void *(void *)> get) noexcept :
        _parent(std::move(parent)), _path(parent->_path + name), _get(std::move(get_lambda))
    {
    }

    void *get(void *state_value) const noexcept
    {
        return _get(_parent->get(state_value));
    }

    [[nodiscard]] constexpr std::string const &path() const noexcept
    {
        return _path;
    }

    [[nodiscard]] friend bool operator==(shared_state_path const& lhs, shared_state_path const& rhs) noexcept
    {
        return lhs._path == rhs._path;
    }

    [[nodiscard]] friend auto operator<=>(shared_state_path const& lhs, shared_state_path const& rhs) noexcept
    {
        return lhs._path <=> rhs._path;
    }

private:
    std::shared_ptr<shared_state_path> _parent;
    std::string _path;
    std::function<void *(void *)> _get;
};

template<typename ParentValueType>
std::shared_ptr<shared_state_path>
make_shared_state_path_name(std::shared_ptr<shared_state_path> parent, member_offset offset, std::string_view name) noexcept
{
    return std::make_shared<shared_state_path>(std::move(parent), std::format(".{}", name), [](void *parent_value) -> void * {
        return advance_bytes(parent_value, offset);
    });
}

template<typename ParentValueType, typename Index>
std::shared_ptr<shared_state_path> make_shared_state_path_index(std::shared_ptr<shared_state_path> parent, Index index) noexcept
{
    return std::make_shared<shared_state_path>(std::move(parent), std::format("[{}]", index), [](void *parent_value) -> void * {
        return std::addressof((*static_cast<ParentValueType *> parent_value)[index]);
    });
}

} // namespace hi::inline v1
