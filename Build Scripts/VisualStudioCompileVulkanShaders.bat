
set PROJECT_DIR=%~1
set TARGET_DIR=%~2

glslangValidator -V -o "%TARGET_DIR%BackingPipeline_vulkan.vert.spv" "%PROJECT_DIR%TTauri\GUI\BackingPipeline_vulkan.vert"
glslangValidator -V -o "%TARGET_DIR%BackingPipeline_vulkan.frag.spv" "%PROJECT_DIR%TTauri\GUI\BackingPipeline_vulkan.frag"

