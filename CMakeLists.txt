cmake_minimum_required(VERSION 3.20)

project(Snake LANGUAGES CXX)

set(APP_NAME Snake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Sources
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_executable(${APP_NAME} ${APP_SOURCES})

target_include_directories(${APP_NAME}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# SDL3
add_subdirectory(external/SDL)

# Dear ImGui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")

target_sources(${APP_NAME}
    PRIVATE
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"

        "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp"
)

target_include_directories(${APP_NAME}
    PRIVATE
        "${IMGUI_DIR}"
        "${IMGUI_DIR}/backends"

        "${CMAKE_CURRENT_BINARY_DIR}"
)

target_link_libraries(${APP_NAME} PRIVATE SDL3::SDL3)
