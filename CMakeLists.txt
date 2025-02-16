cmake_minimum_required(VERSION 3.8)
project("r3d")

if(POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Main project check

set(R3D_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(R3D_IS_MAIN ON)
else()
    set(R3D_IS_MAIN OFF)
endif()

# Configure external dependencies

add_subdirectory("${R3D_ROOT_PATH}/external/raylib")

# Configure r3d library

include("${R3D_ROOT_PATH}/embedded/shaders/EmbedShaders.cmake")
include("${R3D_ROOT_PATH}/embedded/textures/EmbedTextures.cmake")

EmbedShaders(R3D_GENERATED_SHADERS_SOURCE
    "${R3D_ROOT_PATH}/src/embedded/r3d_shaders.c.in"
    "${R3D_ROOT_PATH}/src/embedded/r3d_shaders.h"
)

EmbedTextures(R3D_GENERATED_TEXTURES_SOURCE
    "${R3D_ROOT_PATH}/src/embedded/r3d_textures.c.in"
    "${R3D_ROOT_PATH}/src/embedded/r3d_textures.h"
)

add_library(${PROJECT_NAME}
    "${R3D_ROOT_PATH}/src/details/r3d_primitives.c"
    "${R3D_ROOT_PATH}/src/details/r3d_drawcall.c"
    "${R3D_ROOT_PATH}/src/r3d_environment.c"
    "${R3D_ROOT_PATH}/src/r3d_lighting.c"
    "${R3D_ROOT_PATH}/src/r3d_frustum.c"
    "${R3D_ROOT_PATH}/src/r3d_skybox.c"
    "${R3D_ROOT_PATH}/src/r3d_utils.c"
    "${R3D_ROOT_PATH}/src/r3d_state.c"
    "${R3D_ROOT_PATH}/src/r3d_core.c"
    "${R3D_GENERATED_SHADERS_SOURCE}"
    "${R3D_GENERATED_TEXTURES_SOURCE}"
)

target_link_libraries(${PROJECT_NAME} PUBLIC raylib)

target_include_directories(${PROJECT_NAME} PUBLIC
    "${R3D_ROOT_PATH}/include"
    "${R3D_ROOT_PATH}/external/glad"
    "${R3D_ROOT_PATH}/external/raylib/src"
)

if(CMAKE_VERSION VERSION_GREATER 3.12)
	set_property(TARGET r3d PROPERTY C_STANDARD 99)
endif()

# Examples

add_executable(r3d_basic "examples/basic.c")
target_link_libraries(r3d_basic PRIVATE raylib r3d)
target_include_directories(r3d_basic PRIVATE "external/raylib/src" "include")

add_executable(r3d_pbr "examples/pbr.c")
target_link_libraries(r3d_pbr PRIVATE raylib r3d)
target_compile_definitions(r3d_pbr PRIVATE RESOURCES_PATH="${R3D_ROOT_PATH}/examples/resources/")
target_include_directories(r3d_pbr PRIVATE "external/raylib/src" "include")

add_executable(r3d_skybox "examples/skybox.c")
target_link_libraries(r3d_skybox PRIVATE raylib r3d)
target_compile_definitions(r3d_skybox PRIVATE RESOURCES_PATH="${R3D_ROOT_PATH}/examples/resources/")
target_include_directories(r3d_skybox PRIVATE "external/raylib/src" "include")

add_executable(r3d_sponza "examples/sponza.c")
target_link_libraries(r3d_sponza PRIVATE raylib r3d)
target_compile_definitions(r3d_sponza PRIVATE RESOURCES_PATH="${R3D_ROOT_PATH}/examples/resources/")
target_include_directories(r3d_sponza PRIVATE "external/raylib/src" "include")

add_executable(r3d_fog "examples/fog.c")
target_link_libraries(r3d_fog PRIVATE raylib r3d)
target_compile_definitions(r3d_fog PRIVATE RESOURCES_PATH="${R3D_ROOT_PATH}/examples/resources/")
target_include_directories(r3d_fog PRIVATE "external/raylib/src" "include")

add_executable(r3d_bloom "examples/bloom.c")
target_link_libraries(r3d_bloom PRIVATE raylib r3d)
target_compile_definitions(r3d_bloom PRIVATE RESOURCES_PATH="${R3D_ROOT_PATH}/examples/resources/")
target_include_directories(r3d_bloom PRIVATE "external/raylib/src" "include")
