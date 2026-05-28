#include <cstdint>
#include <iostream>
#include <memory>
#include <openssl/err.h>
#include <signal.h>
#include "../shared/socket.hpp"

// add basic file handling
// game data
// launch scripts/options
// users??
// igdb api metadata
// file compression
// folder compression
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
        std::string msg(data.begin(), data.end());
        std::cout << msg << "\n";
        std::cout << "closing client\n";
        client->close();
        std::cout << "client closed\n";
    }

    SSL_CTX_free(ctx);
    return 0;
}