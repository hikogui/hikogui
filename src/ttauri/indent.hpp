

#pragma once

namespace tt {

/** Indentation for writing out text files.
 * This type is used to pass indentation information recursivly to
 * add indentation to text files, like for example json.
 */
class indent {
public:
    /** Constructor
     * This constructor will start indentation at depth 0.
     *
     * @param spaces Number of spaces per indentation.
     * @param space Character used for indentation.
     */
    [[nodiscard]] constexpr indent(int spaces = 4, char space = ' ') noexcept :
        _space(space), _spaces(spaces), _depth(0) {}

    [[nodiscard]] constexpr indent(indent const &other) noexcept :
        _space(other._space), _spaces(other._spaces), _depth(other._depth) {}

    [[nodiscard]] constexpr indent(indent &&other) noexcept :
        _space(other._space), _spaces(other._spaces), _depth(other._depth) {}

    [[nodiscard]] constexpr indent &operator=(indent const &other) noexcept
    {
        _space = other._space;
        _spaces = other._spaces;
        _depth = other._depth;
        return *this;
    }

    [[nodiscard]] constexpr indent &operator=(indent &&other) noexcept
    {
        _space = other._space;
        _spaces = other._spaces;
        _depth = other._depth;
        return *this;
    }

    /** String conversion operator.
     */
    [[nodiscard]] operator std::string () const noexcept
    {
        return std::string(_depth * _spaces, _space);
    }

    /** Increase the depth of this indentation.
     */
    constexpr indent &operator+=(int rhs) noexcept
    {
        _depth += rhs;
        return *this;
    }

    /** Increment the depth of this indentation.
     */
    constexpr indent &operator++() noexcept
    {
        ++_depth;
        return *this;
    }

    /** Get an indentation at increased depth.
     */
    [[nodiscard]] constexpr friend indent operator+(indent lhs, int rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }

private:
    char _space;
    int _spaces;
    int _depth;
};


}

