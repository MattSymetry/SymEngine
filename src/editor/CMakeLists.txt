cmake_minimum_required(VERSION 3.8)
project(SymEngineEditor)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/src/engine)

file(GLOB_RECURSE EDITOR_SOURCES "*.cpp")
file(GLOB_RECURSE EDITOR_HEADERS "*.h" "*.hpp")

add_executable(SymEngineEditor ${EDITOR_SOURCES} ${EDITOR_HEADERS})

#target_link_libraries(SymEngineEditor SymEngine)

target_link_libraries(SymEngineEditor glm imgui ImGuizmo)

target_link_libraries(SymEngineEditor Vulkan::Vulkan sdl2)

target_include_directories(SymEngineEditor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common
)