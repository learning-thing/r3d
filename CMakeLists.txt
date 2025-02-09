cmake_minimum_required(VERSION 3.8)
project("r3d")

if(POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

add_subdirectory("external/raylib")

add_library(${PROJECT_NAME} "src/r3d.c")
target_link_libraries(${PROJECT_NAME} PUBLIC raylib)
target_include_directories(${PROJECT_NAME} PUBLIC "external/raylib/src")

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET r3d PROPERTY C_STANDARD 99)
endif()
