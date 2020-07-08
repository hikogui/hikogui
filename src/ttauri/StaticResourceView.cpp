// Copyright 2019 Pokitec
// All rights reserved.

#include "StaticResourceView.hpp"
#include "globals.hpp"

namespace tt {

StaticResourceView::StaticResourceView(std::string const &filename) :
    _bytes(getStaticResource(filename))
{
}


}
