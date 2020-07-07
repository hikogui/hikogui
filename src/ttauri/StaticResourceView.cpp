// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/StaticResourceView.hpp"
#include "ttauri/globals.hpp"

namespace tt {

StaticResourceView::StaticResourceView(std::string const &filename) :
    _bytes(getStaticResource(filename))
{
}


}
