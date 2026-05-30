#include "file_transfer_socket.hpp"
#include "socket.hpp"
#include <memory>
#include <stdexcept>

FileTransfer::FileTransfer(std::string ip, uint32_t port, SSL_CTX* ctx) {
    this->socket = std::make_unique<TLSSocket>(ip, port, ctx);
}
void FileTransfer::sendFile (std::filesystem::path path) {
    if(!socket->connect(10)) {
        throw std::runtime_error("Failed to connect socket for file transfer!");
    }
    // file transfer protocol here
}
void FileTransfer::recvFile () {
   socket->listen(1);
   socket->accept();
   // file transfer protocol here
}