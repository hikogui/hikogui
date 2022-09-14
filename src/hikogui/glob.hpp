// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "algorithm.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <ostream>

namespace hi::inline v1 {

enum class glob_callback_token_t {
    String,
    StringList,
    CharacterList,
    InverseCharacterList,
    Separator,
    AnyString,
    AnyCharacter,
    AnyDirectory
};

inline std::ostream &operator<<(std::ostream &lhs, glob_callback_token_t const &rhs)
{
    switch (rhs) {
    case glob_callback_token_t::String: lhs << "String"; break;
    case glob_callback_token_t::StringList: lhs << "StringList"; break;
    case glob_callback_token_t::CharacterList: lhs << "CharacterList"; break;
    case glob_callback_token_t::InverseCharacterList: lhs << "InverseCharacterList"; break;
    case glob_callback_token_t::Separator: lhs << "Separator"; break;
    case glob_callback_token_t::AnyString: lhs << "AnyString"; break;
    case glob_callback_token_t::AnyCharacter: lhs << "AnyCharacter"; break;
    case glob_callback_token_t::AnyDirectory: lhs << "AnyDirectory"; break;
    default: hi_no_default();
    }
    return lhs;
}

struct glob_token_t {
    glob_callback_token_t type;
    std::string value;
    std::vector<std::string> values;

    glob_token_t(glob_callback_token_t type) : type(type), value(), values() {}
    glob_token_t(glob_callback_token_t type, std::string value) : type(type), value(value), values() {}
    glob_token_t(glob_callback_token_t type, std::vector<std::string> values) : type(type), value(), values(values) {}
};

using glob_token_list_t = std::vector<glob_token_t>;
using glob_token_iterator = glob_token_list_t::iterator;
using glob_token_const_iterator = glob_token_list_t::const_iterator;

inline bool operator==(glob_token_t const &lhs, glob_token_t const &rhs) noexcept
{
    return lhs.type == rhs.type && lhs.value == rhs.value && lhs.values == rhs.values;
}

inline std::ostream &operator<<(std::ostream &lhs, glob_token_t const &rhs)
{
    lhs << rhs.type;
    if (rhs.value.size() > 0) {
        lhs << ":" << rhs.value;
    } else if (rhs.values.size() > 0) {
        lhs << ":{";
        for (std::size_t i = 0; i < rhs.values.size(); i++) {
            if (i != 0) {
                lhs << ",";
            }
            lhs << rhs.values[i];
        }
        lhs << "}";
    }
    return lhs;
}

/*! Parse a glob pattern.
 * A glob pattern is designed to match with paths and uses '/' as path separators.
 * The following place holders will be handled:
 *  - '*' matches zero or more characters within a filename or directory name.
 *  - '**' matches zero or more characters in a path, including path separators.
 *  - '?' matches one character.
 *  - '[\<range\>]' matches one character inside the range.
 *  - '[^\<range\>]' matches one character that is not within the range, the path separator '/'
 *    is implicitly included in \<range\>.
 *  - '{\<list\>}' matches one string in the list. The list is a comma ',' separated list
 *    of strings.
 *
 * The following patterns can be part of a \<range\>:
 *  - '-' A dash as the first or last character in \<range\> matches the '-' character.
 *  - ']' A close bracket as the first character in \<range\> matches the ']' character.
 *  - '\<code-unit\>-\<code-unit\>' A dash between two UTF-8 code-units matches all UTF-8 code units
 *    between and including the two given code-units.
 *  - '\<code-unit\>' Matches the UTF-8 code unit itself.
 */
inline glob_token_list_t parseGlob(std::string_view glob)
{
    enum class state_t {
        Idle,
        FoundText,
        FoundSlash,
        FoundEscape,
        FoundSlashStar,
        FoundSlashDoubleStar,
        FoundBracket,
        FoundBrace,
    };
    state_t state = state_t::Idle;

    glob_token_list_t r;
    std::string tmpString;
    std::vector<std::string> tmpStringList;
    bool isInverse = false;
    bool isFirstCharacter = false;
    bool isRange = false;

    auto i = glob.begin();
    while (true) {
        auto c = (i != glob.end()) ? *i : '\0';

        switch (state) {
        case state_t::Idle:
            switch (c) {
            case '/': state = state_t::FoundSlash; break;
            case '?': r.emplace_back(glob_callback_token_t::AnyCharacter); break;
            case '*': r.emplace_back(glob_callback_token_t::AnyString); break;
            case '[':
                isInverse = false;
                isFirstCharacter = true;
                isRange = false;
                state = state_t::FoundBracket;
                break;
            case '{': state = state_t::FoundBrace; break;
            case '\\': state = state_t::FoundEscape; break;
            case '\0': return r;
            default: state = state_t::FoundText; continue;
            }
            break;

        case state_t::FoundText:
            if (c == '/' || c == '?' || c == '*' || c == '[' || c == '{' || c == '\0') {
                r.emplace_back(glob_callback_token_t::String, tmpString);
                tmpString.clear();
                state = state_t::Idle;
                continue; // Don't increment the iterator.
            } else if (c == '\\') {
                state = state_t::FoundEscape;
            } else {
                tmpString += c;
            }
            break;

        case state_t::FoundEscape:
            if (c == '\0') {
                r.emplace_back(glob_callback_token_t::String, tmpString);
                state = state_t::Idle;
                continue; // Don't increment the iterator.
            } else {
                tmpString += c;
                state = state_t::FoundText;
            }
            break;

        case state_t::FoundSlash:
            if (c == '*') {
                state = state_t::FoundSlashStar;
            } else {
                r.emplace_back(glob_callback_token_t::Separator);
                state = state_t::Idle;
                continue;
            }
            break;

        case state_t::FoundSlashStar:
            if (c == '*') {
                state = state_t::FoundSlashDoubleStar;
            } else {
                r.emplace_back(glob_callback_token_t::Separator);
                r.emplace_back(glob_callback_token_t::AnyString);
                state = state_t::Idle;
                continue;
            }
            break;

        case state_t::FoundSlashDoubleStar:
            if (c == '/') {
                r.emplace_back(glob_callback_token_t::AnyDirectory);
                r.emplace_back(glob_callback_token_t::Separator);
                state = state_t::Idle;

            } else {
                // Fallback to AnyString, as if there was only a single '*'.
                r.emplace_back(glob_callback_token_t::Separator);
                r.emplace_back(glob_callback_token_t::AnyString);
                state = state_t::Idle;
                continue; // Don't increment the iterator.
            }
            break;

        case state_t::FoundBracket:
            switch (c) {
            case '^':
                if (isFirstCharacter) {
                    isInverse = true;
                    tmpString += '/';
                } else {
                    tmpString += c;
                }
                break;

            case ']':
                if (isFirstCharacter) {
                    tmpString += c;
                } else {
                    if (isRange) {
                        tmpString += '-';
                    }

                    if (isInverse) {
                        r.emplace_back(glob_callback_token_t::InverseCharacterList, tmpString);
                    } else {
                        r.emplace_back(glob_callback_token_t::CharacterList, tmpString);
                    }

                    tmpString.clear();
                    state = state_t::Idle;
                }
                isFirstCharacter = false;
                break;

            case '-':
                if (isFirstCharacter) {
                    tmpString += '-';
                } else {
                    isRange = true;
                }
                isFirstCharacter = false;
                break;

            case '\0':
                if (isRange) {
                    tmpString += '-';
                }

                if (isInverse) {
                    r.emplace_back(glob_callback_token_t::InverseCharacterList, tmpString);
                } else {
                    r.emplace_back(glob_callback_token_t::CharacterList, tmpString);
                }
                state = state_t::Idle;
                continue; // Don't increment the iterator.

            default:
                if (isRange) {
                    hilet firstCharacter = static_cast<uint8_t>(tmpString.back());
                    hilet lastCharacter = static_cast<uint8_t>(c);
                    for (uint8_t tmp_c = firstCharacter + 1; tmp_c <= lastCharacter; tmp_c++) {
                        tmpString += static_cast<char>(tmp_c);
                    }
                } else {
                    tmpString += c;
                }
                isRange = false;
                isFirstCharacter = false;
                break;
            }
            break;

        case state_t::FoundBrace:
            switch (c) {
            case '}':
                tmpStringList.push_back(tmpString);
                tmpString.clear();
                r.emplace_back(glob_callback_token_t::StringList, tmpStringList);
                tmpStringList.clear();
                state = state_t::Idle;
                break;
            case ',':
                tmpStringList.push_back(tmpString);
                tmpString.clear();
                break;
            case '\0':
                tmpStringList.push_back(tmpString);
                r.emplace_back(glob_callback_token_t::StringList, tmpStringList);
                state = state_t::Idle;
                continue; // Don't increment the iterator.
            default: tmpString += c; break;
            }
            break;

        default: hi_no_default();
        }

        i++;
    }
}

enum class glob_match_result_t { No, Partial, Match };

inline glob_match_result_t matchGlob(glob_token_const_iterator index, glob_token_const_iterator end, std::string_view str)
{
    if (index == end) {
        return (str.size() == 0) ? glob_match_result_t::Match : glob_match_result_t::No;

    } else if (str.size() == 0) {
        switch (index->type) {
        case glob_callback_token_t::Separator: return glob_match_result_t::Partial;
        case glob_callback_token_t::AnyDirectory: return glob_match_result_t::Partial;
        case glob_callback_token_t::AnyString: return matchGlob(index + 1, end, str);
        default: return glob_match_result_t::No;
        }
    }

#define MATCH_GLOB_RECURSE(out, next, end, str) \
    switch (hilet tmp = matchGlob(next, end, str)) { \
    case glob_match_result_t::No: break; \
    case glob_match_result_t::Match: return tmp; \
    case glob_match_result_t::Partial: out = tmp; break; \
    default: hi_no_default(); \
    }

    // result may be assigned Partial by MATCH_GLOB_RECURSE.
    auto result = glob_match_result_t::No;
    bool found_slash = false;
    hilet next_index = index + 1;

    switch (index->type) {
    case glob_callback_token_t::String:
        if (str.starts_with(index->value)) {
            MATCH_GLOB_RECURSE(result, next_index, end, str.substr(index->value.size()));
        }
        return result;

    case glob_callback_token_t::StringList:
        for (hilet &value : index->values) {
            if (str.starts_with(value)) {
                MATCH_GLOB_RECURSE(result, next_index, end, str.substr(value.size()));
            }
        }
        return result;

    case glob_callback_token_t::CharacterList:
        if (index->value.find(str.front()) != std::string::npos) {
            MATCH_GLOB_RECURSE(result, next_index, end, str.substr(1));
        }
        return result;

    case glob_callback_token_t::InverseCharacterList:
        if (index->value.find(str.front()) == std::string::npos) {
            MATCH_GLOB_RECURSE(result, next_index, end, str.substr(1));
        }
        return result;

    case glob_callback_token_t::Separator:
        if (str.front() == '/') {
            return matchGlob(next_index, end, str.substr(1));
        } else {
            return glob_match_result_t::No;
        }

    case glob_callback_token_t::AnyCharacter:
        if (str.front() != '/') {
            return matchGlob(next_index, end, str.substr(1));
        } else {
            return glob_match_result_t::No;
        }

    case glob_callback_token_t::AnyString:
        // Loop through each character in the string, including the end.
        for (std::size_t i = 0; i <= str.size(); i++) {
            MATCH_GLOB_RECURSE(result, next_index, end, str.substr(i));

            // Don't continue beyond a slash.
            if (i < str.size() && str[i] == '/') {
                break;
            }
        }
        return result;

    case glob_callback_token_t::AnyDirectory:
        // Recurse after each slash.
        found_slash = false;
        for (std::size_t i = 0; i <= str.size(); i++) {
            if (i == str.size() || str[i] == '/') {
                MATCH_GLOB_RECURSE(result, next_index, end, str.substr(i));
            }
        }
        return result;

    default: hi_no_default();
    }
#undef MATCH_GLOB_RECURSE
}

inline glob_match_result_t matchGlob(glob_token_list_t const &glob, std::string_view str)
{
    return matchGlob(glob.begin(), glob.end(), str);
}

inline glob_match_result_t matchGlob(std::string_view glob, std::string_view str)
{
    return matchGlob(parseGlob(glob), str);
}

inline std::string basePathOfGlob(glob_token_const_iterator first, glob_token_const_iterator last)
{
    if (first == last) {
        return "";
    }

    // Find the first place holder and don't include it as a token.
    auto endOfBase = std::find_if_not(first, last, [](hilet &x) {
        return x.type == glob_callback_token_t::String || x.type == glob_callback_token_t::Separator;
    });

    if (endOfBase != last) {
        // Backtrack until the last separator, and remove it.
        // Except when we included everything in the first loop because in that case there
        // are no placeholders at all and we want to include the filename.
        endOfBase = rfind_if(first, endOfBase, [](hilet &x) {
            return x.type == glob_callback_token_t::Separator;
        });
    }

    // Add back the leading slash.
    if (endOfBase == first && first->type == glob_callback_token_t::Separator) {
        endOfBase++;
    }

    std::string r;
    for (auto index = first; index != endOfBase; index++) {
        switch (index->type) {
        case glob_callback_token_t::String: r += index->value; break;
        case glob_callback_token_t::Separator: r += '/'; break;
        default: hi_no_default();
        }
    }
    return r;
}

inline std::string basePathOfGlob(glob_token_list_t const &glob)
{
    return basePathOfGlob(glob.begin(), glob.end());
}

inline std::string basePathOfGlob(std::string_view glob)
{
    return basePathOfGlob(parseGlob(glob));
}

} // namespace hi::inline v1
