//
//  Application.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <memory>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>

namespace TTauri {
namespace Toolkit {

class Application {
public:
    static Application *shared;

    boost::filesystem::path resourceDir;

    boost::filesystem::path getPathToResource(const boost::filesystem::path &resource) {
        return boost::filesystem::canonical(resource, resourceDir);
    }

    Application(const boost::filesystem::path &resourceDir);
    ~Application();
};

extern std::shared_ptr<Application> app;

}}
