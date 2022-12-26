

#pragma once

namespace hi {
inline namespace v1 {

template<typename It, typename EndIt>
class lexer {
public:
    class const_iterator {
    public:
        constexpr const_iterator(lexer *lexer) noexcept : _lexer(lexer), _it(lexer->_begin), _end(lexer->_end)
        {
            parse_token();
        }

        constexpr const_iterator &operator++() const noexcept
        {
            parse_token();
            return *this;
        }

        [[nodiscard]] constexpr bool operator==(std::default_sentinel_t) const noexcept
        {
            return _finished
        }

    private:
        It _it;
        EndIt _end;

        lexer *_lexer = nullptr;
        bool _finished = false;

        constexpr parse_token() noexcept
        {
            if (_it == lexer->_last) {
                finished = true;
                return;
            }

            hilet [c, utf8_success] = char_map<"utf-8">{}.read(_it, _end);

            if (is_start_id(c)
            if ((c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_' )
                parse_id

        }
    };


    constexpr lexer(It first, EndIt last) noexcept : _first(first), _last(last) {}

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return const_iterator{this};
    }

    [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept
    {
        return {};
    }


private:
    It _begin;
    EndIt _end; 
};

}}

