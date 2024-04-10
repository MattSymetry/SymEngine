cmake_minimum_required (VERSION 3.8)

project ("SymEngine")

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
  glslang
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG        5755de46b07e4374c05fb1081f65f7ae1f8cca81 # Use the specific commit you mentioned
)

FetchContent_MakeAvailable(glslang)

add_subdirectory(third_party)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/$<CONFIG>)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/bin/${OUTPUTCONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/bin/${OUTPUTCONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/bin/${OUTPUTCONFIG})
endforeach()

add_subdirectory(src/engine)
add_subdirectory(src/editor)


find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

file(GLOB_RECURSE GLSL_SOURCE_FILES "${PROJECT_SOURCE_DIR}/shaders/*.comp")
source_group(TREE "${PROJECT_SOURCE_DIR}/shaders" FILES ${GLSL_SOURCE_FILES})
add_custom_target(Shaders SOURCES ${GLSL_SOURCE_FILES})

file(GLOB_RECURSE ASSETS_SOURCE_FILES "${PROJECT_SOURCE_DIR}/assets/*")

foreach(GLSL ${GLSL_SOURCE_FILES})
  list(APPEND GLSL_FILES ${GLSL})
endforeach(GLSL)

foreach(ASSET ${ASSETS_SOURCE_FILES})
  list(APPEND ASSET_FILES ${ASSET})
endforeach(ASSET)

set(GLSL_FILES ${GLSL_FILES} CACHE INTERNAL "List of SPIR-V binary files")
set(ASSET_FILES ${ASSET_FILES} CACHE INTERNAL "List of assets")