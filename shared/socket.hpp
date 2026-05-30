#pragma once
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <vector>
#include <openssl/ssl.h>

enum class SOCKET_TYPE { TCP, UDP, TLS };

class Socket {
    protected:
    int32_t socket_num;
    uint32_t port;
    std::string ip;
    char* recv_buffer = nullptr;
    public:
    SOCKET_TYPE type;
    Socket (std::string ip, uint32_t port, SOCKET_TYPE type);
    ~Socket() {
        if (recv_buffer) {
            delete[] recv_buffer;
        }
    }
    virtual bool connect(int retry_count = 0) = 0;
    virtual bool close();
    virtual bool send(void* data, uint32_t size) = 0;
    virtual std::vector<uint8_t> recv(bool block) = 0;
};

class TCPSocket : public Socket {
    public:
    TCPSocket (std::string ip, uint32_t port) : Socket(ip, port, SOCKET_TYPE::TCP) {
        this->socket_num = socket(AF_INET, SOCK_STREAM, 0);
    }
    TCPSocket(int fd);
    bool connect(int retry_count = 0);
    bool bind();
    bool listen(int backlog);
    std::unique_ptr<TCPSocket> accept();
    bool send(void* data, uint32_t size);
    std::vector<uint8_t> recv(bool block);
};
// convert to using BIO??
class TLSSocket : public TCPSocket {
    private:
    SSL_CTX* ctx;
    SSL* ssl;
    public:
    TLSSocket(int fd, SSL_CTX* ctx);
    TLSSocket (std::string ip, uint32_t port, SSL_CTX* ctx) : TCPSocket(ip, port) {
        this->type = SOCKET_TYPE::TLS;
        this->ctx = ctx;
        this->ssl = SSL_new(ctx);
        SSL_set_fd(this->ssl, this->socket_num);
    }
    ~TLSSocket () {
        if (this->ssl) {
            SSL_shutdown(ssl);
            SSL_free(this->ssl);
            this->ssl = nullptr;
        }
    }
    // move and copy
    TLSSocket (const TLSSocket& sock) : TCPSocket(sock.socket_num) {
        this->type = sock.type;
        this->ctx = sock.ctx;
        this->ssl = SSL_new(ctx);
        SSL_set_fd(this->ssl, this->socket_num);
    }
    std::unique_ptr<TLSSocket> accept();
    bool connect(int retry_count = 0);
    bool close();
    bool send(void* data, uint32_t size);
    std::vector<uint8_t> recv(bool block);
};