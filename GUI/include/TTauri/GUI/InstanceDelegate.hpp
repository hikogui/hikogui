// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri::GUI {

class InstanceDelegate {
public:
    virtual void lastWindowClosed() = 0;
};

}