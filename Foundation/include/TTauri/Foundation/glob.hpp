// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <ostream>

namespace TTauri {

enum class glob_token_name_t {
    Choice,
    AnyString,
    AnyCharacter,
    AnyDirectory
};

inline std::ostream &operator<<(std::ostream &lhs, glob_token_name_t const &rhs) {
    switch (rhs) {
    case glob_token_name_t::Choice: lhs << "Choice"; break;
    case glob_token_name_t::AnyString: lhs << "AnyString"; break;
    case glob_token_name_t::AnyCharacter: lhs << "AnyCharacter"; break;
    case glob_token_name_t::AnyDirectory: lhs << "AnyDirectory"; break;
    default: no_default;
    }
    return lhs;
}

struct glob_token_t {
    glob_token_name_t type;
    std::vector<std::string> values;

    glob_token_t(glob_token_name_t type) : type(type), values() {}
    glob_token_t(glob_token_name_t type, std::string value) : type(type), values({value}) {}
    glob_token_t(glob_token_name_t type, std::vector<std::string> values) : type(type), values(values) {}
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

std::vector<glob_token_t> parseGlobPattern(std::string_view glob)
{
    enum class state_t {
        Idle,
        FoundText,
        FoundEscape,
        FoundQuestion,
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
            case '\\': state = state_t::FoundEscape; break;
            case '*': state = state_t::FoundStar; break;
            case '?': state = state_t::FoundQuestion; break;
            case '[': state = state_t::FoundBracket; break;
            case '{': state = state_t::FoundBrace; break;
            case '\0': return r;
            default: state = state_t::FoundText; continue;
            }
            break;

        case state_t::FoundText:
            if (c == '*' || c == '?' || c == '[' || c == ']' || c == '{' || c == '\0') {
                r.emplace_back(glob_token_name_t::Choice, tmpString);
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
                r.emplace_back(glob_token_name_t::Choice, tmpString);
                state = state_t::Idle;
                continue; // Don't increment the iterator.
            } else {
                tmpString += c;
                state = state_t::FoundText;
            }
            break;

        case state_t::FoundQuestion:
            state = state_t::Idle;
            r.emplace_back(glob_token_name_t::AnyCharacter);
            continue;

        case state_t::FoundStar:
            state = state_t::Idle;
            if (c == '*') {
                r.emplace_back(glob_token_name_t::AnyDirectory);
            } else {
                r.emplace_back(glob_token_name_t::AnyString);
                continue; // Don't increment the iterator.
            }
            break;

        case state_t::FoundBracket:
            switch (c) {
            case ']':
                r.emplace_back(glob_token_name_t::Choice, tmpChoice);
                tmpChoice.clear();
                state = state_t::Idle;
                break;
            case '\0':
                r.emplace_back(glob_token_name_t::Choice, tmpChoice);
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
                r.emplace_back(glob_token_name_t::Choice, tmpChoice);
                tmpChoice.clear();
                state = state_t::Idle;
                break;
            case ',':
                tmpChoice.push_back(tmpString);
                tmpString.clear();
                break;
            case '\0':
                tmpChoice.push_back(tmpString);
                r.emplace_back(glob_token_name_t::Choice, tmpChoice);
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

}
