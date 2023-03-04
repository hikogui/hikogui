// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gfx_device.hpp"
#include "gfx_device_vulkan.hpp"
#include "gfx_draw_context.hpp"
#include "gfx_queue_vulkan.hpp"
#include "gfx_surface.hpp"
#include "gfx_surface_delegate.hpp"
#include "gfx_surface_delegate_vulkan.hpp"
#include "gfx_system.hpp"
#include "gfx_system_globals.hpp"
#include "gfx_system_vulkan.hpp"
#include "paged_image.hpp"
#include "pipeline.hpp"
#include "pipeline_alpha.hpp"
#include "pipeline_alpha_device_shared.hpp"
#include "pipeline_alpha_push_constants.hpp"
#include "pipeline_alpha_vertex.hpp"
#include "pipeline_box.hpp"
#include "pipeline_box_device_shared.hpp"
#include "pipeline_box_push_constants.hpp"
#include "pipeline_box_vertex.hpp"
#include "pipeline_image.hpp"
#include "pipeline_image_device_shared.hpp"
#include "pipeline_image_push_constants.hpp"
#include "pipeline_image_texture_map.hpp"
#include "pipeline_image_vertex.hpp"
#include "pipeline_SDF.hpp"
#include "pipeline_SDF_device_shared.hpp"
#include "pipeline_SDF_push_constants.hpp"
#include "pipeline_SDF_texture_map.hpp"
#include "pipeline_SDF_vertex.hpp"
#include "pipeline_tone_mapper.hpp"
#include "pipeline_tone_mapper_device_shared.hpp"
#include "pipeline_tone_mapper_push_constants.hpp"
#include "pipeline_vulkan.hpp"
#include "RenderDoc.hpp"
#include "renderdoc_app.h"
#include "subpixel_orientation.hpp"

namespace hi { inline namespace v1 {
/**
\defgroup GFX Graphics
\ingroup GUI
\brief Operating System interface to low-level drawing and the GPU.

*/
}} // namespace hi::v1
