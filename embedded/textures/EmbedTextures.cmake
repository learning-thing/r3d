find_program(PYTHON_EXECUTABLE python3 REQUIRED)

function(ProcessTexture binary_file var_name)
    # Get the file size
    file(SIZE "${binary_file}" file_size)
    # Convert the file to hexadecimal
    execute_process(
        COMMAND ${PYTHON_EXECUTABLE} "${R3D_ROOT_PATH}/scripts/bin2c.py" "${binary_file}"
        OUTPUT_VARIABLE hex_output
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # Set the variables
    set("${var_name}" "${hex_output}" PARENT_SCOPE)
    set("${var_name}_SIZE" "${file_size}" PARENT_SCOPE)
endfunction()

function(EmbedTextures generated_textures_source textures_source textures_header)
    # Convert textures to C-Hex stream
    ProcessTexture("${R3D_ROOT_PATH}/embedded/textures/iblBrdfLut.dds" TEX_IBL_BRDF_LUT)
    # Generate shaders source file
    set(GENERATED_TEXTURES_SOURCE "${CMAKE_BINARY_DIR}/generated/src/embedded/r3d_textures.c")
    set(${generated_textures_source} "${GENERATED_TEXTURES_SOURCE}" PARENT_SCOPE)
    configure_file("${textures_source}" "${GENERATED_TEXTURES_SOURCE}")
endfunction()
