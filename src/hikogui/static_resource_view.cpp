// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "static_resource_view.hpp"
#include "static_resource_list.hpp"
#include "exception.hpp"

namespace hi::inline v1 {

static_resource_view::static_resource_view(std::string const &filename) :
    _bytes(static_resource_view::get_static_resource(filename))
{
}

const_void_span static_resource_view::get_static_resource(std::string const &filename)
{
    auto r = static_resource_item::find(filename);
    if (r.empty()) {
        throw key_error(std::format("Could not find static resource '{}'.", filename));
    }
    return r;
}

} // namespace hi::inline v1
