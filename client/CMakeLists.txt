cmake_minimum_required(VERSION 3.25)

project(rom_pack_client LANGUAGES CXX)

include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_PIPEWIRE OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.9
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL2)

set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_11_1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(curl)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.9b
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

find_package(OpenGL REQUIRED)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${sdl2_SOURCE_DIR}/include
)

target_link_libraries(imgui PUBLIC
    SDL2::SDL2-static
    OpenGL::GL
)

add_executable(client ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp
    request.hpp
    request.cpp)
target_link_libraries(client PRIVATE
    imgui
    CURL::libcurl
    nlohmann_json::nlohmann_json
)
