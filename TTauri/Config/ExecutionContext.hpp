
#pragma once

#include "Value.hpp"

namespace TTauri::Config {

struct ExecutionContext {
    std::vector<std::shared_ptr<ValueBase>> objectStack;
};

}