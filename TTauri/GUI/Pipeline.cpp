// Copyright 2019 Pokitec
// All rights reserved.

#include "Pipeline.hpp"

namespace TTauri::GUI {

using namespace std;

Pipeline::Pipeline(const std::shared_ptr<Window> window) :
    window(std::move(window))
{
}

}
