// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/StaticResourceView.hpp"
#include "ttauri/foundation/globals.hpp"

namespace tt {

StaticResourceView::StaticResourceView(std::string const &filename) :
    _bytes(getStaticResource(filename))
{
}


}
