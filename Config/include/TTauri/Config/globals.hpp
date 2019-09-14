// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri::Config {

struct ConfigGlobals;
inline ConfigGlobals *Config_globals = nullptr;

struct ConfigGlobals {
public:
    ConfigGlobals();
    ~ConfigGlobals();
    ConfigGlobals(ConfigGlobals const &) = delete;
    ConfigGlobals &operator=(ConfigGlobals const &) = delete;
    ConfigGlobals(ConfigGlobals &&) = delete;
    ConfigGlobals &operator=(ConfigGlobals &&) = delete;
};

}