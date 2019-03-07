
set PROJECT_DIR=%~1
set TARGET_DIR=%~2

glslangValidator -V -o "%TARGET_DIR%BackingPipeline.vert.spv" "%PROJECT_DIR%TTauri\GUI\BackingPipeline.vert"
glslangValidator -V -o "%TARGET_DIR%BackingPipeline.frag.spv" "%PROJECT_DIR%TTauri\GUI\BackingPipeline.frag"

