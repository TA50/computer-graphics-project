glslc shaders/BlinnShader.frag -o shaders/BlinnFrag.spv
glslc shaders/BlinnShader.vert -o shaders/BlinnVert.spv

@REM "shaders/EmissionVert.spv", "shaders/EmissionFrag.spv
glslc shaders/EmissionShader.frag -o shaders/EmissionFrag.spv
glslc shaders/EmissionShader.vert -o shaders/EmissionVert.spv

@REM  "shaders/SkyBoxVert.spv", "shaders/SkyBoxFrag.spv",

glslc shaders/SkyBoxShader.frag -o shaders/SkyBoxFrag.spv
glslc shaders/SkyBoxShader.vert -o shaders/SkyBoxVert.spv

@REM "shaders/NormalMapVert.spv", "shaders/NormalMapFrag.spv"
glslc shaders/NormalMapShader.frag -o shaders/NormalMapFrag.spv
glslc shaders/NormalMapShader.vert -o shaders/NormalMapVert.spv

@REM "shaders/TextShader.

glslc shaders/TextShader.frag -o shaders/TextFrag.spv
glslc shaders/TextShader.vert -o shaders/TextVert.spv





@REM Loop through all the files in the shaders directory and compile them
for %%f in (assets\shaders\src\*) do (
    glslc %%f -o assets\shaders\src\%%~nf.spv
)