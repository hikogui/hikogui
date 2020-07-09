// Copyright 2019 Pokitec
// All rights reserved.

#include "StaticResourceView.hpp"
#include "Application.hpp"

namespace tt {

StaticResourceView::StaticResourceView(std::string const &filename) :
    _bytes(application->getStaticResource(filename))
{
}


}
