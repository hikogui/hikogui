
#pragma once

#include "Value.hpp"

namespace TTauri::Config {

struct ExecutionContext {
    std::vector<Value> objectStack;
};

}