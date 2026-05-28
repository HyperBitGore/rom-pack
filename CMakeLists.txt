cmake_minimum_required(VERSION 3.14)
project(ServerClient VERSION 1.0)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(ALSA REQUIRED)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.9
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

FetchContent_Declare(
    g_engine_2d
    GIT_REPOSITORY https://github.com/HyperBitGore/g_engine_2d.git
    GIT_TAG        main
)
FetchContent_GetProperties(g_engine_2d)
if(NOT g_engine_2d_POPULATED)
    FetchContent_Populate(g_engine_2d)
endif()

add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

add_library(g_engine_2d_lib STATIC
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/audio/audio_player.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/engine_modern.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/engine_start.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/font_renderer.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/function_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/image_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/input.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/matrix.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/shader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/window.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/util/vector.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/primitive_renderer.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/image_renderer.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/image_atlas.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/bmp_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/font_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/img_loading/png_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/file_reader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/rendering/wireframe_renderer.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/model_loading/obj_loader.cpp
    ${g_engine_2d_SOURCE_DIR}/include/g_engine/file_loading/model_loading/model.cpp
)
target_include_directories(g_engine_2d_lib PUBLIC
    ${g_engine_2d_SOURCE_DIR}/include
    ${g_engine_2d_SOURCE_DIR}/include/GL
    ${X11_INCLUDE_DIR}
    ${X11_Xkb_INCLUDE_PATH}
)
target_link_libraries(g_engine_2d_lib PUBLIC OpenGL::GL dl X11::X11 X11::Xkb ALSA::ALSA)

# Server executable
add_executable(server
    server/file.hpp
    server/file.cpp
    server/server.cpp
    server/server_files.cpp
    server/server_files.hpp
    shared/socket.cpp
)
target_link_libraries(server OpenSSL::SSL OpenSSL::Crypto)

# Client executable
add_executable(client
    client/client.cpp
    shared/socket.cpp
)
target_link_libraries(client OpenSSL::SSL OpenSSL::Crypto g_engine_2d_lib imgui_lib)