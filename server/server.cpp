#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <openssl/err.h>
#include <signal.h>
#include "../shared/socket.hpp"
#include "../shared/socket_enums.hpp"
#include "server_files.hpp"
// move socket reply functions to seperate thread
//  - thread pool???
// game data
//  - add library syncing between server and client
//      - client library just the shape of library, download files when want to
//  - upload folder
// launch scripts/options
//  - map where a emulator/launcher is on for local client
// users??
//      - utilize some identifier in every packet??
// consume zip files and consume them
//      - decompress and recompress with our superior method
// igdb api metadata
// file compression
// folder compression

FileManager fm;

bool helloMsg (std::vector<uint8_t>& data, std::unique_ptr<TLSSocket>& sock) {
    if (data.empty() || data[0] != std::to_underlying(SocketConnectType::HELLO)) {
        return false;
    }
    std::string username(data.begin() + 1, data.end());
    std::cout << "Hello from: " << username << "\n";
    std::vector<uint8_t> response = { std::to_underlying(SocketConnectType::HELLO) };
    return sock->send(&response[0], response.size());
}
bool uploadMsg (std::vector<uint8_t>& data, std::unique_ptr<TLSSocket>& sock) {
    // expect: [FILE_UPLOAD_BEGIN (1B), file_size (8B LE), filename bytes]
    if (data.size() < 10 || data[0] != std::to_underlying(SocketConnectType::FILE_UPLOAD_BEGIN)) {
        return false;
    }
    uint64_t file_size = (uint64_t)data[1]
                       | ((uint64_t)data[2] << 8)
                       | ((uint64_t)data[3] << 16)
                       | ((uint64_t)data[4] << 24)
                       | ((uint64_t)data[5] << 32)
                       | ((uint64_t)data[6] << 40)
                       | ((uint64_t)data[7] << 48)
                       | ((uint64_t)data[8] << 56);
    std::string file_name(data.begin() + 9, data.end());
    uint32_t id;
    try {
        uint32_t id = fm.addIncomingFile(file_name, file_size);
    } catch (std::runtime_error e) {
        std::cerr << e.what() << "\n";
        return false;
    }
    uint16_t window_size = 1024;

    std::vector<uint8_t> response = { std::to_underlying(SocketConnectType::FILE_UPLOAD_BEGIN) };
    // id as 4 bytes little-endian
    response.push_back(id & 0xFF);
    response.push_back((id >> 8) & 0xFF);
    response.push_back((id >> 16) & 0xFF);
    response.push_back((id >> 24) & 0xFF);
    // window_size as 2 bytes little-endian
    response.push_back(window_size & 0xFF);
    response.push_back((window_size >> 8) & 0xFF);

    return sock->send(&response[0], response.size());
}

bool uploadFolderMsg (std::vector<uint8_t>& data, std::unique_ptr<TLSSocket>& sock) {

    return true;
}


bool uploadBlock (std::vector<uint8_t>& data, std::unique_ptr<TLSSocket>& sock) {
    if (data.size() < 5 || data[0] != std::to_underlying(SocketConnectType::FILE_UPLOAD_BLOCK)) {
        return false;
    }
    uint32_t id = (uint64_t)data[1]
                       | ((uint64_t)data[2] << 8)
                       | ((uint64_t)data[3] << 16)
                       | ((uint64_t)data[4] << 24);
    std::vector<uint8_t> block(data.begin() + 5, data.end());
    fm.updateIncomingFile(id, block);    
    return true;
}

bool sendCategories (std::vector<uint8_t>& data, std::unique_ptr<TLSSocket>& sock) {
    if (data.size() < 5 || data[0] != std::to_underlying(SocketConnectType::CATEGORIES)) {
        return false;
    }
    std::vector<uint8_t> block = fm.serialize();
    std::vector<uint8_t> buffer = {std::to_underlying(SocketConnectType::CATEGORIES)};
    for (auto& i : block) {
        buffer.push_back(i);
    }
    sock->send(&buffer[0], buffer.size());
    return true;
}

int main() {
    SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
    if (!SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM)) {
        std::cerr << "Failed to load cert.pem\n";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 1;
    }
    if (!SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)) {
        std::cerr << "Failed to load key.pem\n";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 1;
    }
    SSL_CTX_set_num_tickets(ctx, 0);
    signal(SIGPIPE, SIG_IGN);
    fm.addFileLibrary();
    TLSSocket server("", 9001, ctx);
    if (!server.bind()) {
        std::cout << "Failed to bind socket\n";
        SSL_CTX_free(ctx);
        return 1;
    }
    server.listen(10);
    std::cout << "Server listening on port 9001\n";
    while (true) {
        std::unique_ptr<TLSSocket> client = server.accept();
        std::cout << "client accepted\n";
        if (!client) {
            std::cout << "client failed to be accepted\n";
            continue;
        }
        std::cout << "Client connected\n";
        std::vector<uint8_t> data = client->recv(true);
        if (data.size() > 0) {
            switch (data[0]) {
                case std::to_underlying(SocketConnectType::HELLO):
                    if (!helloMsg(data, client)) {
                        std::cout << "Hello handshake failed\n";
                    }
                break;
                case std::to_underlying(SocketConnectType::FILE_UPLOAD_BEGIN):
                    if (!uploadMsg(data, client)) {
                        std::cout << "Upload handshake failed\n";
                    }
                break;
                case std::to_underlying(SocketConnectType::FOLDER_UPLOAD_BEGIN):
                    if (!uploadFolderMsg(data, client)) {
                        std::cout << "Upload folder handshake failed\n";
                    }
                case std::to_underlying(SocketConnectType::FILE_UPLOAD_BLOCK):
                    if (!uploadBlock(data, client)) {
                        std::cout << "Block failed!\n";
                    }
                break;
                case std::to_underlying(SocketConnectType::CATEGORIES):
                    if (!sendCategories(data, client)) {
                        std::cout << "Categories send failed!\n";
                    }
                break;
            }
        }
        std::cout << "closing client\n";
        client->close();
        std::cout << "client closed\n";
    }

    SSL_CTX_free(ctx);
    return 0;
}