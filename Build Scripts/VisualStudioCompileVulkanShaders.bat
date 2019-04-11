
set PROJECT_DIR=%~1
set TARGET_DIR=%~2

glslangValidator -V -o "%TARGET_DIR%PipelineImage.vert.spv" "%PROJECT_DIR%TTauri\GUI\PipelineImage.vert"
glslangValidator -V -o "%TARGET_DIR%PipelineImage.frag.spv" "%PROJECT_DIR%TTauri\GUI\PipelineImage.frag"

copy "%PROJECT_DIR%TTauri\GUI\camera.png" "%TARGET_DIR%camera.png"
