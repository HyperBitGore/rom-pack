#include <cstdio>

#include <SDL.h>
#include <SDL_opengl.h>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "database.hpp"
#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_sdl2.h"
#include "current_page.hpp"
#include "pages.hpp"

// Bridge for gui elements?, elements themselves are decorated hierachy, Bridge is just a blob of them
// Pages just use virtual classes and a unique_ptr in main, factory function that populates it??

// TODO
//  - export game pack
//      - need support for game types with no path just the raw data saved in pack
//  - memmap db file instead of mass saving/loading?
//  - Server
//      - Login
//      - Pack syncing
//  - game images
//  - rework popups to be more inline with element inheritance

static void apply_theme() {
    ImGui::StyleColorsDark();
    auto& style = ImGui::GetStyle();
    style.WindowPadding = {24, 20};
    style.FramePadding = {12, 8};
    style.ItemSpacing = {12, 10};
    style.FrameRounding = 2.0f;
    style.PopupRounding = 5.0f;

    auto& colors = style.Colors;
    colors[ImGuiCol_WindowBg] = {0.06f, 0.08f, 0.11f, 1.0f};
    colors[ImGuiCol_PopupBg] = {0.10f, 0.13f, 0.17f, 1.0f};
    colors[ImGuiCol_FrameBg] = {0.14f, 0.18f, 0.23f, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = {0.18f, 0.30f, 0.34f, 1.0f};
    colors[ImGuiCol_FrameBgActive] = {0.18f, 0.36f, 0.40f, 1.0f};
    colors[ImGuiCol_Button] = {0.14f, 0.24f, 0.28f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = {0.18f, 0.36f, 0.40f, 1.0f};
    colors[ImGuiCol_ButtonActive] = {0.21f, 0.43f, 0.47f, 1.0f};
    colors[ImGuiCol_Header] = colors[ImGuiCol_Button];
    colors[ImGuiCol_HeaderHovered] = colors[ImGuiCol_ButtonHovered];
    colors[ImGuiCol_HeaderActive] = colors[ImGuiCol_ButtonActive];
    colors[ImGuiCol_TitleBg] = colors[ImGuiCol_PopupBg];
    colors[ImGuiCol_TitleBgActive] = colors[ImGuiCol_FrameBg];
    colors[ImGuiCol_CheckMark] = {0.44f, 0.82f, 0.78f, 1.0f};
    colors[ImGuiCol_Separator] = {0.24f, 0.36f, 0.40f, 1.0f};

    auto* fonts = ImGui::GetIO().Fonts;
    for (const char* path : {
        "/usr/share/fonts/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    }) {
        std::error_code error;
        if (std::filesystem::is_regular_file(path, error) &&
            fonts->AddFontFromFileTTF(path, 17.0f)) {
            return;
        }
    }
    ImFontConfig font;
    font.SizePixels = 17.0f;
    fonts->AddFontDefault(&font);
}

int main(int, char**) {
    game_db& db = game_db::instance();
    try {
        db.load();
    } catch (std::runtime_error e) {
        std::cerr << e.what() << "\n";
    }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(
        "ROM-Pack",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (window == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        std::fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    apply_theme();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");
    {
        current_page& current_page = current_page::instance();
        current_page.set_page(pages::constructGameListPage());
    }
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                running = false;
            }
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE) {
                running = false;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        current_page& current_page = current_page::instance();
        current_page.render();
        ImGui::Render();
        int display_width = 0;
        int display_height = 0;
        SDL_GL_GetDrawableSize(window, &display_width, &display_height);
        glViewport(0, 0, display_width, display_height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        if (!current_page.run) {
            break;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
