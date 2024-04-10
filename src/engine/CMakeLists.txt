cmake_minimum_required(VERSION 3.8)
project(EngineLibrary)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h" "*.hpp")

source_group("Shader" FILES ${GLSL_SOURCE_FILES})
add_executable(SymEngine ${SOURCES} ${HEADERS} ${GLSL_SOURCE_FILES})

set(ASSETS_DIR "${PROJECT_SOURCE_DIR}/../../assets")

foreach(ASSET_FILE ${ASSET_FILES})
    file(RELATIVE_PATH ASSET_REL_PATH ${ASSETS_DIR} ${ASSET_FILE})
    set(OUTPUT_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/${ASSET_REL_PATH}")

    add_custom_command(TARGET SymEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy ${ASSET_FILE} ${OUTPUT_PATH}
        COMMENT "Copying ${ASSET_FILE} | ${ASSETS_DIR}  to ${OUTPUT_PATH}")
endforeach()

foreach(GLSL_FILE ${GLSL_FILES})
    get_filename_component(FILE_NAME ${GLSL_FILE} NAME)
    set(OUTPUT_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/${FILE_NAME}")
    add_custom_command(TARGET SymEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy ${GLSL_FILE} ${OUTPUT_PATH}
        COMMENT "Copying ${GLSL_FILE} to ${OUTPUT_PATH}")
endforeach()

set_property(TARGET SymEngine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:SymEngine>")

target_include_directories(SymEngine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src/common"
)

target_link_libraries(SymEngine vma glm imgui glslang SPIRV OGLCompiler HLSL)


target_link_libraries(SymEngine Vulkan::Vulkan sdl2)

add_dependencies(SymEngine Shaders)