#include "socket.hpp"
#include <filesystem>
#include <memory>

class FileTransfer {
    private:
        std::unique_ptr<TLSSocket> socket = nullptr;
    public:
        FileTransfer(std::string ip, uint32_t port, SSL_CTX* ctx);
        void sendFile (std::filesystem::path path);
        void recvFile ();
};