

#pragma once

hi_export_module(hikogui.theme : style_importance);

hi_export namespace hi::inline v1 {


struct style_importance : uint8_t {
    initial = 0,

    user = 1,

    theme = 2,

    author = 3,

    important_author = 4,

    important_theme = 5,

    important_user = 6,

    important_initial = 7
};

[[nodiscard]] constexpr style_importance make_important(style_importance rhs) noexcept
{
    switch (rhs) {
    case initial:
        return important_initial;
    case user:
        return important_user;
    case theme:
        return important_theme;
    case author:
        return important_author;
    default:
        std::unreachable();
    }
}

}
