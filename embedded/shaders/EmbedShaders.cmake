find_program(PYTHON_EXECUTABLE python3 REQUIRED)

function(ProcessShader shader_file var_name)
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} "${R3D_ROOT_PATH}/scripts/glsl_minifier.py" "${shader_file}"
        OUTPUT_VARIABLE shader_output
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set("${var_name}" "${shader_output}" PARENT_SCOPE)
endfunction()

function(EmbedShaders generated_shaders_source shaders_source shaders_header)
    # Minify and store common shaders
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/common/screen.vs.glsl" VS_COMMON_SCREEN)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/common/cubemap.vs.glsl" VS_COMMON_CUBEMAP)
    # Minify and store generate shaders
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/generate/gaussianBlurDualPass.fs.glsl" FS_GENERATE_GAUSSIAN_BLUR_DUAL_PASS)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/generate/cubemapFromEquirectangular.fs.glsl" FS_GENERATE_CUBEMAP_FROM_EQUIRECTANGULAR)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/generate/irradianceConvolution.fs.glsl" FS_GENERATE_IRRADIANCE_CONVOLUTION)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/generate/prefilter.fs.glsl" FS_GENERATE_PREFILTER)
    # Minify and store raster shaders
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/geometry.vs.glsl" VS_RASTER_GEOMETRY)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/geometry.inst.vs.glsl" VS_RASTER_GEOMETRY_INST)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/geometry.fs.glsl" FS_RASTER_GEOMETRY)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/skybox.vs.glsl" VS_RASTER_SKYBOX)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/skybox.fs.glsl" FS_RASTER_SKYBOX)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/depth.vs.glsl" VS_RASTER_DEPTH)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/depth.vs.inst.glsl" VS_RASTER_DEPTH_INST)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/depth.fs.glsl" FS_RASTER_DEPTH)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/depthCube.vs.glsl" VS_RASTER_DEPTH_CUBE)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/depthCube.inst.vs.glsl" VS_RASTER_DEPTH_CUBE_INST)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/raster/depthCube.fs.glsl" FS_RASTER_DEPTH_CUBE)
    # Minify and store screen shaders
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/screen/ssao.fs.glsl" FS_SCREEN_SSAO)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/screen/lighting.fs.glsl" FS_SCREEN_LIGHTING)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/screen/bloom.fs.glsl" FS_SCREEN_BLOOM)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/screen/fog.fs.glsl" FS_SCREEN_FOG)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/screen/tonemap.fs.glsl" FS_SCREEN_TONEMAP)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/screen/adjustment.fs.glsl" FS_SCREEN_ADJUSTMENT)
    ProcessShader("${R3D_ROOT_PATH}/embedded/shaders/screen/fxaa.fs.glsl" FS_SCREEN_FXAA)
    # Generate shaders source file
    set(GENERATED_SHADERS_SOURCE "${CMAKE_BINARY_DIR}/generated/src/embedded/r3d_shaders.c")
    set(${generated_shaders_source} "${GENERATED_SHADERS_SOURCE}" PARENT_SCOPE)
    configure_file("${shaders_source}" "${GENERATED_SHADERS_SOURCE}")
endfunction()
