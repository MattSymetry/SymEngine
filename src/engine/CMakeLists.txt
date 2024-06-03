cmake_minimum_required(VERSION 3.8)
project(EngineLibrary)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h" "*.hpp")

source_group("Shader" FILES ${GLSL_SOURCE_FILES})
add_executable(SYMYS ${SOURCES} ${HEADERS} ${PROJECT_SOURCE_DIR}/../../resources/resource.rc)

if (WIN32)
    target_sources(SYMYS PRIVATE ${PROJECT_SOURCE_DIR}/../../resources/resource.rc)
endif()

set_property(TARGET SYMYS PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:SYMYS>")

target_include_directories(SYMYS PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src/common"
)

target_link_libraries(SYMYS vma glm imgui ImGuizmo glslang SPIRV OGLCompiler HLSL cereal tinyfiledialogs)


target_link_libraries(SYMYS Vulkan::Vulkan sdl2)

add_dependencies(SYMYS Shaders)