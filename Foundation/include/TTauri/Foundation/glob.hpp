// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/algorithm.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <ostream>

namespace TTauri {

enum class glob_token_type_t {
    Choice,
    Seperator,
    AnyString,
    AnyCharacter,
    AnyDirectory
};

inline std::ostream &operator<<(std::ostream &lhs, glob_token_type_t const &rhs) {
    switch (rhs) {
    case glob_token_type_t::Choice: lhs << "Choice"; break;
    case glob_token_type_t::Seperator: lhs << "Seperator"; break;
    case glob_token_type_t::AnyString: lhs << "AnyString"; break;
    case glob_token_type_t::AnyCharacter: lhs << "AnyCharacter"; break;
    case glob_token_type_t::AnyDirectory: lhs << "AnyDirectory"; break;
    default: no_default;
    }
    return lhs;
}

struct glob_token_t {
    glob_token_type_t type;
    std::vector<std::string> values;

    glob_token_t(glob_token_type_t type) : type(type), values() {}
    glob_token_t(glob_token_type_t type, std::string value) : type(type), values({value}) {}
    glob_token_t(glob_token_type_t type, std::vector<std::string> values) : type(type), values(values) {}
};

inline bool operator==(glob_token_t const &lhs, glob_token_t const &rhs) noexcept {
    return lhs.type == rhs.type && lhs.values == rhs.values;
}

inline std::ostream &operator<<(std::ostream &lhs, glob_token_t const &rhs) {
    lhs << rhs.type;
    if (rhs.values.size() == 1) {
        lhs << ":" << rhs.values[0];
    } else if (rhs.values.size() > 1) {
        lhs << ":{";
        for (size_t i = 0; i < rhs.values.size(); i++) {
            if (i != 0) {
                lhs << ",";
            }
            lhs << rhs.values[i];
        }
        lhs << "}";
    }
    return lhs;
}

std::vector<glob_token_t> parseGlob(std::string_view glob)
{
    enum class state_t {
        Idle,
        FoundText,
        FoundEscape,
        FoundStar,
        FoundBracket,
        FoundBrace,
    };
    state_t state = state_t::Idle;

    std::vector<glob_token_t> r;
    std::string tmpString;
    std::vector<std::string> tmpChoice;

    auto i = glob.begin();
    while (true) {
        auto c = (i != glob.end()) ? *i : '\0';

        switch (state) {
        case state_t::Idle:
            switch (c) {
            case '/': r.emplace_back(glob_token_type_t::Seperator); break;
            case '?': r.emplace_back(glob_token_type_t::AnyCharacter); break;
            case '*': state = state_t::FoundStar; break;
            case '[': state = state_t::FoundBracket; break;
            case '{': state = state_t::FoundBrace; break;
            case '\\': state = state_t::FoundEscape; break;
            case '\0': return r;
            default: state = state_t::FoundText; continue;
            }
            break;

        case state_t::FoundText:
            if (c == '/' || c == '?' || c == '*' || c == '[' || c == '{' || c == '\0') {
                r.emplace_back(glob_token_type_t::Choice, tmpString);
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
                r.emplace_back(glob_token_type_t::Choice, tmpString);
                state = state_t::Idle;
                continue; // Don't increment the iterator.
            } else {
                tmpString += c;
                state = state_t::FoundText;
            }
            break;

        case state_t::FoundStar:
            state = state_t::Idle;
            if (c == '*') {
                r.emplace_back(glob_token_type_t::AnyDirectory);
            } else {
                r.emplace_back(glob_token_type_t::AnyString);
                continue; // Don't increment the iterator.
            }
            break;

        case state_t::FoundBracket:
            switch (c) {
            case ']':
                r.emplace_back(glob_token_type_t::Choice, tmpChoice);
                tmpChoice.clear();
                state = state_t::Idle;
                break;
            case '\0':
                r.emplace_back(glob_token_type_t::Choice, tmpChoice);
                state = state_t::Idle;
                continue; // Don't increment the iterator.
            default:
                tmpChoice.emplace_back(1, c);
                break;
            }
            break;

        case state_t::FoundBrace:
            switch (c) {
            case '}':
                tmpChoice.push_back(tmpString);
                tmpString.clear();
                r.emplace_back(glob_token_type_t::Choice, tmpChoice);
                tmpChoice.clear();
                state = state_t::Idle;
                break;
            case ',':
                tmpChoice.push_back(tmpString);
                tmpString.clear();
                break;
            case '\0':
                tmpChoice.push_back(tmpString);
                r.emplace_back(glob_token_type_t::Choice, tmpChoice);
                state = state_t::Idle;
                continue; // Don't increment the iterator.
            default:
                tmpString += c;
                break;
            }
            break;

        default:
            no_default;
        }

        i++;
    }
}

enum class glob_match_result_t {
    No,
    Partial,
    Match
};

using glob_iterator = std::vector<glob_token_t>::iterator;

inline glob_match_result_t matchGlob(glob_iterator index, glob_iterator end, std::string_view str)
{
    if (index == end) {
        return (str.size() == 0) ?
            glob_match_result_t::Match :
            glob_match_result_t::No;

    } else if (str.size() == 0) {
        switch (index->type) {
        case glob_token_type_t::Seperator:
            return glob_match_result_t::Partial;
        case glob_token_type_t::AnyString:
        case glob_token_type_t::AnyDirectory:
            return matchGlob(index + 1, end, str);
        default:
            return glob_match_result_t::No;
        }
    }

#define MATCH_GLOB_RECURSE(out, next, end, str)\
    switch (let tmp = matchGlob(next, end, str)) {\
    case glob_match_result_t::No: break;\
    case glob_match_result_t::Match: return tmp;\
    case glob_match_result_t::Partial: out = tmp; break;\
    default: no_default;\
    }

    // result may be assigned Partial by MATCH_GLOB_RECURSE.
    auto result = glob_match_result_t::No;

    switch (index->type) {
    case glob_token_type_t::Choice:
        for (let value: index->values) {
            if (starts_with(str, value)) {
                MATCH_GLOB_RECURSE(result, index + 1, end, str.substr(value.size()));
            }
        }
        return result;

    case glob_token_type_t::Seperator:
        if (str.front() == '/') {
            return matchGlob(index+1, end, str.substr(1));
        } else {
            return glob_match_result_t::No;
        }

    case glob_token_type_t::AnyCharacter:
        if (str.front() != '/') {
            return matchGlob(index+1, end, str.substr(1));
        } else {
            return glob_match_result_t::No;
        }

    case glob_token_type_t::AnyString:
        // Loop through each character in the string, including the end.
        for (size_t i = 0; i <= str.size(); i++) {
            MATCH_GLOB_RECURSE(result, index + 1, end, str.substr(i));

            // Don't continue beyond a slash.
            if (i < str.size() && str[i] == '/') {
                break;
            }
        }
        return result;

    case glob_token_type_t::AnyDirectory:
        // Loop through each character in the string, including the end.
        for (size_t i = 0; i <= str.size(); i++) {
            MATCH_GLOB_RECURSE(result, index + 1, end, str.substr(i));
        }
        return result;

    default:
        no_default;
    }
#undef MATCH_GLOB_RECURSE
}

inline glob_match_result_t matchGlob(std::vector<glob_token_t> glob, std::string_view str)
{
    return matchGlob(glob.begin(), glob.end(), str);
}

inline glob_match_result_t matchGlob(std::string_view glob, std::string_view str)
{
    let pattern = parseGlob(glob);
    return matchGlob(pattern, str);
}

}
