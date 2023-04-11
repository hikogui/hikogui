
#pragma once

namespace hi {
inline namespace v1 {

class spell_errors {
public:
    class error_type {
    public:
        language_tag language;
        size_t offset;
        size_t size;

        /** Get the replacements for this error.
         *
         * @retval empty Suggest to delete the error.
         * @retval single-string Suggest a single replacement word.
         * @retval multiple-string Suggest a multiple replacement words.
         */
        [[nodiscard]] std::vector<gstring> suggestions() const noexcept;

    private:
        spell_errors *_ptr;

        error_type(spell_errors *self, size_t offset, size_t size) noexcept : offset(offset), size(size), _ptr(self) {}
    };

private:
    std::vector<error_type> _errors;
};


[[nodiscard]] spell_errors check_spelling(gstring text, language_tag language, size_t offset = 0) noexcept;

template<typename It, std::sentinal_for<It> ItEnd>
[[nodiscard]] spell_errors check_spelling(It first, ItEnd last) noexcept
    requires (std::is_same_v<std::iter_value_t<It>, character>)
{
    if (first == last) {
        return;
    }

    auto r = spell_errors{};

    auto run_language = first->attributes().language_tag();
    auto run_start = first;
    for (auto it = first + 1; it != last; ++it) {
        auto language = it->attributes().language_tag();
        if (language != run_language) {
            //r += check_spelling(to_gstring(text),
        }
    }


    return r;
}

}}

