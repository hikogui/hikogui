
#pragma once

namespace hi {
inline namespace v1 {

class spell_errors {
public:
    class error_type {
    public:
        size_t index;
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

        error_type(spell_errors *self, size_t index, size_t size) noexcept : index(index), size(size), _ptr(self) {}
    };

private:
    std::vector<error_type> _errors;
};


[[nodiscard]] spell_errors check_spelling(gstring text, language_tag tag) noexcept;

[[nodiscard]] spell_errors check_spelling(hi::text text) noexcept;

}}

