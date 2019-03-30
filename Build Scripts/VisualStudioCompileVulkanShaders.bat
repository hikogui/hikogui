
set PROJECT_DIR=%~1
set TARGET_DIR=%~2

glslangValidator -V -o "%TARGET_DIR%PipelineRectanglesFromAtlas.vert.spv" "%PROJECT_DIR%TTauri\GUI\PipelineRectanglesFromAtlas.vert"
glslangValidator -V -o "%TARGET_DIR%PipelineRectanglesFromAtlas.frag.spv" "%PROJECT_DIR%TTauri\GUI\PipelineRectanglesFromAtlas.frag"

