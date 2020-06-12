// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/StaticResourceView.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace tt {

StaticResourceView::StaticResourceView(std::string const &filename) :
    _bytes(getStaticResource(filename))
{
}


}
