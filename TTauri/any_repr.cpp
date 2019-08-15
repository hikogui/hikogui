// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "any_repr.hpp"
#include "required.hpp"
#include "URL.hpp"
#include <fmt/format.h>
#include <typeinfo>
#include <any>

namespace TTauri {

std::string any_repr(std::any boxed_value)
{
    let &boxed_type = boxed_value.type();
    if (boxed_type == typeid(void)) {
        return "void";
    } else if (boxed_type == typeid(char)) {
        return fmt::format("{}", std::any_cast<char>(boxed_value));
    } else if (boxed_type == typeid(uint8_t)) {
        return fmt::format("{}", std::any_cast<uint8_t>(boxed_value));
    } else if (boxed_type == typeid(uint16_t)) {
        return fmt::format("{}", std::any_cast<uint16_t>(boxed_value));
    } else if (boxed_type == typeid(uint32_t)) {
        return fmt::format("{}", std::any_cast<uint32_t>(boxed_value));
    } else if (boxed_type == typeid(uint64_t)) {
        return fmt::format("{}", std::any_cast<uint64_t>(boxed_value));
    } else if (boxed_type == typeid(int8_t)) {
        return fmt::format("{}", std::any_cast<int8_t>(boxed_value));
    } else if (boxed_type == typeid(int16_t)) {
        return fmt::format("{}", std::any_cast<int16_t>(boxed_value));
    } else if (boxed_type == typeid(int32_t)) {
        return fmt::format("{}", std::any_cast<int32_t>(boxed_value));
    } else if (boxed_type == typeid(int64_t)) {
        return fmt::format("{}", std::any_cast<int64_t>(boxed_value));
    } else if (boxed_type == typeid(std::string)) {
        return fmt::format("\"{}\"", std::any_cast<std::string>(boxed_value));
    } else if (boxed_type == typeid(char const *)) {
        return fmt::format("\"{}\"", std::any_cast<char const *>(boxed_value));
    } else if (boxed_type == typeid(char *)) {
        return fmt::format("\"{}\"", std::any_cast<char *>(boxed_value));
    } else if (boxed_type == typeid(URL)) {
        return fmt::format("{}", to_string(std::any_cast<URL>(boxed_value)));
    }

    return fmt::format("<{}>", boxed_type.name());
}

}