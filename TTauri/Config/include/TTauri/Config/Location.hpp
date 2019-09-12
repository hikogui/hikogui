// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/URL.hpp"
#include "TTauri/datum.hpp"
#include "TTauri/exceptions.hpp"
#include <fmt/format.h>
#include <memory>
#include <iostream>

namespace TTauri::Config {

/*! Location inside a configuration file.
 */
struct Location {
    std::shared_ptr<URL> file;
    int line;
    int column;

    Location() noexcept : file({}), line(0), column(0) {}

    Location(std::shared_ptr<URL> const &file, int line, int column) noexcept : file(file), line(line), column(column) {}

    explicit Location(datum const &d) :
        file({}), line(0), column(0)
    {
        if (d.is_vector()) {
            let v = static_cast<datum::vector>(d);
            if (v.size() == 3 && gsl::at(v, 0).is_url() && gsl::at(v, 1).is_integer() && gsl::at(v, 2).is_integer()) {
                file = std::make_shared<URL>(static_cast<URL>(gsl::at(v, 0)));
                line = static_cast<int>(gsl::at(v, 1));
                column = static_cast<int>(gsl::at(v, 2));
                return;
            }
        }
        TTAURI_THROW(invalid_operation_error("Can not convert {} of type {} to a Location.", d.type_name(), d.repr()));
    }

    operator datum() const noexcept {
        return datum(datum::vector{datum(*file), datum(line), datum(column)});
    }
    
    std::string string() const noexcept {
        return fmt::format("{0}:{1}:{2}", *file, line, column);
    }
};

inline std::ostream& operator<<(std::ostream &os, const Location &l)
{
    os << l.string();
    return os;
}

}


