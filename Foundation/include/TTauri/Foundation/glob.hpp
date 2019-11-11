

#include <vector>
#include <string>
#include <variant>

namespace TTauri {

enum class glob_type_t {
    Choice,
    AnyString,
    AnyCharacter,
    AnyDirectory
};

struct glob_part_t {
    glob_type_t type;
    std::vector<std:string> values;

    glob_part_t(glob_type_t type) : type(type), values() {}
    glob_part_t(glob_type_t type, std::string value) : type(type), values({value}) {}
    glob_part_t(glob_type_t type, std::vector<std::string> values) : type(type), values(values) {}
};

std::vector<glob_part_t> parse_glob(std::string_view glob)
{
    enum class state_t {
        Idle,
        FoundStar,
        FoundBracket,
        FoundBrace,
    };
    state_t state = state_t::Idle;

    std::vector<glob_part_t> r;
    std::string tmpString;

    auto i = glob.begin();
    while (i != glob.end()) {
        auto c = *i;

        switch (state) {
        case state_t::Idle:
            i++;
            switch (c) {
            case '*':
                r.emplace_back(glob_type_t::Choice, tmpString);
                state = state_t:FoundStar;
                break;
            case '?':
                r.emplace_back(glob_type_t::Choice, tmpString);
                r.emplace_back(glob_type_t::AnyCharacter);
                break;
            case '[':
                r.emplace_back(glob_type_t::Choice, tmpString);
                state = state_t:FoundBracket;
                break;
            case '{':
                r.emplace_back(glob_type_t::Choice, tmpString);
                state = state_t:FoundBrace;
                break;
            default:
                tmpString += c;
            }
            break;

        case state_t::FoundStar:
            if (c == '*') {
                i++;
                r.emplace_back(glob_type_t::AnyDirectory);
            } else {
                r.emplace_back(glob_type_t::AnyString);
                state = state_t::Idle;
            }
            break;

        case state_t::FoundBracket:
            i++;
            if (c == ']') {
                r.emplace_back(glob_type_t::AnyChoise, tmpChoice);
                state = state_t::Idle;
            } else {
                tmpChoice.emplace_back(1, c);
            }
            break;

        case state_t::FoundBrace:
            i++;
            switch (c) {
            case '}':
                r.emplace_back(glob_type_t::AnyChoise, tmpChoice);
                state = state_t::Idle;
                break;
            case ',':
                tmpChoise.push_back(tmpString);
                break;
            default:
                tmpString += c;
                break;
            }
            break;

        default:
            no_default;
        }
    }

    return r;
}

}
