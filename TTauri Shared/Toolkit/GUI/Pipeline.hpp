//
//  Pipeline.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <string>
#include <boost/filesystem.hpp>
#include <vulkan/vulkan.hpp>

namespace TTauri {
namespace Toolkit {
namespace GUI {

class Window;
class Device;

class Pipeline {
public:
    Window *window;
    Device *device;
    
    boost::filesystem::path vertexShaderPath;
    boost::filesystem::path fragmentShaderPath;

    Pipeline(Window *window, boost::filesystem::path vertexShaderPath, boost::filesystem::path fragmentShaderPath);
    virtual ~Pipeline();

private:
    vk::ShaderModule loadShader(boost::filesystem::path path);

};

}}}
