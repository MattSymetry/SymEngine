cmake_minimum_required(VERSION 3.8)
project(EngineLibrary)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h" "*.hpp")
file(GLOB_RECURSE GLSL_SOURCE_FILES "${PROJECT_SOURCE_DIR}/shaders/*.comp")
source_group("Shader" FILES ${GLSL_SOURCE_FILES})
add_library(SymEngine SHARED ${SOURCES} ${HEADERS} ${GLSL_SOURCE_FILES})

foreach(SPIRV_FILE ${SPIRV_BINARY_FILES})
    get_filename_component(FILE_NAME ${SPIRV_FILE} NAME)
    set(OUTPUT_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/${FILE_NAME}")
    add_custom_command(TARGET SymEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy ${SPIRV_FILE} ${OUTPUT_PATH}
        COMMENT "Copying ${SPIRV_FILE} to ${OUTPUT_PATH}")
endforeach()

set_property(TARGET SymEngine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:SymEngine>")

target_include_directories(SymEngine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src/common"
)

target_link_libraries(SymEngine vma glm imgui)

target_link_libraries(SymEngine Vulkan::Vulkan sdl2)

add_dependencies(SymEngine Shaders)