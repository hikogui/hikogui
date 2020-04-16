// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/globals.hpp"
#include "TTauri/GUI/Instance.hpp"
#include "TTauri/GUI/ThemeMode.hpp"
#include "TTauri/Text/globals.hpp"
#include "TTauri/Foundation/globals.hpp"

#include "shaders/PipelineImage.vert.spv.inl"
#include "shaders/PipelineImage.frag.spv.inl"
#include "shaders/PipelineFlat.vert.spv.inl"
#include "shaders/PipelineFlat.frag.spv.inl"
#include "shaders/PipelineBox.vert.spv.inl"
#include "shaders/PipelineBox.frag.spv.inl"
#include "shaders/PipelineSDF.vert.spv.inl"
#include "shaders/PipelineSDF.frag.spv.inl"


namespace TTauri::GUI {


void startup()
{
    if (startupCount.fetch_add(1) != 0) {
        // The library has already been initialized.
        return;
    }

    TTauri::startup();
    TTauri::Text::startup();
    LOG_AUDIT("TTauri::GUI startup");

    addStaticResource(PipelineImage_vert_spv_filename, PipelineImage_vert_spv_bytes);
    addStaticResource(PipelineImage_frag_spv_filename, PipelineImage_frag_spv_bytes);
    addStaticResource(PipelineFlat_vert_spv_filename, PipelineFlat_vert_spv_bytes);
    addStaticResource(PipelineFlat_frag_spv_filename, PipelineFlat_frag_spv_bytes);
    addStaticResource(PipelineBox_vert_spv_filename, PipelineBox_vert_spv_bytes);
    addStaticResource(PipelineBox_frag_spv_filename, PipelineBox_frag_spv_bytes);
    addStaticResource(PipelineSDF_vert_spv_filename, PipelineSDF_vert_spv_bytes);
    addStaticResource(PipelineSDF_frag_spv_filename, PipelineSDF_frag_spv_bytes);

    themeMode = readOSThemeMode();
    LOG_INFO("Operating system theme mode is {}", themeMode);

    try {
        keyboardBindings.loadSystemBindings();
    } catch (error &e) {
        LOG_FATAL("Could not load keyboard bindings {}", to_string(e));
    }

    guiSystem = new Instance(guiDelegate);
}

void shutdown()
{
    if (startupCount.fetch_sub(1) != 1) {
        // This is not the last instantiation.
        return;
    }
    LOG_AUDIT("TTauri::GUI shutdown");

    delete guiSystem;
    TTauri::Text::shutdown();
    TTauri::shutdown();
}

}
