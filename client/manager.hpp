#pragma once
#include "../shared/socket_enums.hpp"
#include "../shared/socket.hpp"
#include <mutex>
// managers files and categories on server side

class Manager {
    private:
        std::mutex lib_mut;
        std::vector<Category> library;
        bool display = false;
        // thread function that updates library
        void getLibrary (SSL_CTX* ctx, std::string ip, uint32_t port);
    public:
        Manager ();
        // copy
        Manager (const Manager& m);
        // renders the ImGui
        void render();
        // updates the library, spawns a thread to update
        void updateLibrary (SSL_CTX* ctx, std::string ip, uint32_t port);
        bool getDisplay () {
            return this->display;
        }
        void toggleDisplay () {
            this->display = !this->display;
        }
        std::vector<Category> retrieveLibrary () {
            return library;
        }
        
};