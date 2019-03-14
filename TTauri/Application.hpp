//
//  Application.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "GUI/Instance.hpp"

#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>

#include <memory>
#include <string>

namespace TTauri {

class Application {
public:
    class Delegate {
    public:
        virtual void initialize() = 0;
    };

    struct Error : virtual boost::exception, virtual std::exception {};


    std::shared_ptr<Delegate> delegate;
    std::shared_ptr<GUI::Instance> instance;
    boost::filesystem::path resourceDir;

    Application(std::shared_ptr<Delegate> delegate, std::vector<const char *> vulkanExtensions);
    virtual ~Application();

    virtual void createWindow(std::shared_ptr<GUI::Window::Delegate> windowDelegate, const std::string &title) = 0;
    virtual void initialize();
    virtual int loop() = 0;

    static std::shared_ptr<Application> shared;
};
}
