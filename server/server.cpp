#include <iostream>
#include <openssl/err.h>
#include "../shared/socket.hpp"

// add basic file handling
// users??
// game data
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

    TLSSocket server("", 9001, ctx);
    if (!server.bind()) {
        std::cout << "Failed to bind socket\n";
        SSL_CTX_free(ctx);
        return 1;
    }
    server.listen(10);
    std::cout << "Server listening on port 9001\n";

    while (true) {
        auto client = server.accept();
        if (!client) continue;
        std::cout << "Client connected\n";
        auto data = client->recv(true);
        std::string msg(data.begin(), data.end());
        std::cout << msg << "\n";
        client->close();
    }

    SSL_CTX_free(ctx);
    return 0;
}