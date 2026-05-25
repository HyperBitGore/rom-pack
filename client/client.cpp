#include <iostream>
#include <openssl/err.h>
#include "../shared/socket.hpp"

// https://github.com/immediate-mode-ui/nuklear

int main() {
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    if (!SSL_CTX_load_verify_locations(ctx, "cert.pem", nullptr)) {
        std::cerr << "Failed to load cert.pem\n";
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 1;
    }
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

    TLSSocket client("127.0.0.1", 9001, ctx);
    if (!client.connect(10)) {
        std::cout << "Error connecting to server\n";
        SSL_CTX_free(ctx);
        return 1;
    }
    std::cout << "Connected to server\n";
    const char* msg = "Hello server";
    client.send((void*)msg, sizeof("Hello server"));
    client.close();

    SSL_CTX_free(ctx);
    return 0;
}