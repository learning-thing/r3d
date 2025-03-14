cmake_minimum_required(VERSION 3.8)
project("r3d")

# This section configures MSVC (Visual Studio) to enable "Edit and Continue" debugging

if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Determine the project's root directory and check if it's the main project

set(R3D_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(R3D_IS_MAIN ON)
else()
    set(R3D_IS_MAIN OFF)
endif()

# Project configuration options

option(R3D_BUILD_SHARED "Build r3d as a shared library" OFF)
option(R3D_BUILD_EXAMPLES "Build the examples" ${R3D_IS_MAIN})
option(R3D_BUILD_RAYLIB_SHARED "Build raylib as dynamic if built with r3d" OFF)

set(R3D_RAYLIB_PATH "" CACHE STRING "Path to a specific raylib installation")

# Include standard cmake scripts

include(CheckLibraryExists)

# Checks for raylib installation

if(R3D_BUILD_RAYLIB_SHARED)
    set(BUILD_SHARED_LIBS ON)
endif()

if(NOT "${R3D_RAYLIB_PATH}" STREQUAL "" AND EXISTS "${R3D_RAYLIB_PATH}/CMakeLists.txt")
    message(STATUS "Using custom raylib installation: ${R3D_RAYLIB_PATH}")
    add_subdirectory("${R3D_RAYLIB_PATH}")
elseif(EXISTS "${R3D_ROOT_PATH}/external/raylib/CMakeLists.txt")
    message(STATUS "Using raylib from submodule: ${R3D_ROOT_PATH}/external/raylib")
    add_subdirectory("${R3D_ROOT_PATH}/external/raylib")
else()
    find_package(raylib QUIET)
    if(TARGET raylib)
        message(STATUS "Using system-installed raylib")
    else()
        message(FATAL_ERROR "No valid raylib installation found!
- Custom path: '${R3D_RAYLIB_PATH}' (invalid or missing CMakeLists.txt)
- Submodule: '${R3D_ROOT_PATH}/external/raylib' (missing or incomplete)
- System-wide installation: find_package(raylib) failed

Please provide a valid raylib installation, initialize the submodule with:
    git submodule update --init --recursive
or install raylib on your system.")
    endif()
endif()

# Configure r3d library: Include external resources for shaders and textures

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

# Create the r3d library by adding all the source files

set(R3D_LIB_TYPE STATIC)

if(R3D_BUILD_SHARED)
    set(R3D_LIB_TYPE SHARED)
    add_definitions(-DR3D_BUILD_SHARED)
endif()

add_library(${PROJECT_NAME} ${R3D_LIB_TYPE}
    "${R3D_ROOT_PATH}/src/details/r3d_projection.c"
    "${R3D_ROOT_PATH}/src/details/r3d_primitives.c"
    "${R3D_ROOT_PATH}/src/details/r3d_billboard.c"
    "${R3D_ROOT_PATH}/src/details/r3d_collision.c"
    "${R3D_ROOT_PATH}/src/details/r3d_drawcall.c"
    "${R3D_ROOT_PATH}/src/details/r3d_frustum.c"
    "${R3D_ROOT_PATH}/src/details/r3d_light.c"
    "${R3D_ROOT_PATH}/src/r3d_environment.c"
    "${R3D_ROOT_PATH}/src/r3d_particles.c"
    "${R3D_ROOT_PATH}/src/r3d_lighting.c"
    "${R3D_ROOT_PATH}/src/r3d_culling.c"
    "${R3D_ROOT_PATH}/src/r3d_skybox.c"
    "${R3D_ROOT_PATH}/src/r3d_curves.c"
    "${R3D_ROOT_PATH}/src/r3d_sprite.c"
    "${R3D_ROOT_PATH}/src/r3d_utils.c"
    "${R3D_ROOT_PATH}/src/r3d_state.c"
    "${R3D_ROOT_PATH}/src/r3d_core.c"
    "${R3D_GENERATED_SHADERS_SOURCE}"
    "${R3D_GENERATED_TEXTURES_SOURCE}"
)

# Link project dependencies

target_link_libraries(${PROJECT_NAME} PUBLIC raylib)

check_library_exists(m cos "" HAVE_LIB_M)

if(HAVE_LIB_M)
    target_link_libraries(${PROJECT_NAME} PUBLIC m)
endif()

# Specify the include directories needed for the project

target_include_directories(${PROJECT_NAME} PUBLIC
    "${R3D_ROOT_PATH}/include"
    "${R3D_ROOT_PATH}/external/glad"
    "${R3D_ROOT_PATH}/external/raylib/src"
)

# Ensure the C standard is set for MSVC if using CMake version greater than 3.12

if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET r3d PROPERTY C_STANDARD 99)
endif()

# Example configuration

if(R3D_BUILD_EXAMPLES)
    include("${R3D_ROOT_PATH}/examples/CMakeLists.txt")
endif()
