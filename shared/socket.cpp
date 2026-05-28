#include "socket.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <openssl/err.h>
#include <thread>
#include <unistd.h>

Socket::Socket(std::string ip, uint32_t port, SOCKET_TYPE type) {
    this->port = port;
    this->ip = ip;
    socket_num = -1;
    this->type = type;
    this->recv_buffer = new char [1024];
}
TCPSocket::TCPSocket(int fd) : Socket("", 0, SOCKET_TYPE::TCP) {
    this->socket_num = fd;
}
bool TCPSocket::bind() {
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);
    addr.sin_addr.s_addr = INADDR_ANY;
    int result = ::bind(this->socket_num, (struct sockaddr*)&addr, sizeof(addr));
    return result == 0;
}
bool TCPSocket::listen(int backlog) {
    return ::listen(this->socket_num, backlog) == 0;
}
std::unique_ptr<TCPSocket> TCPSocket::accept() {
    int client_fd = ::accept(this->socket_num, nullptr, nullptr);
    if (client_fd == -1) return nullptr;
    return std::move(std::make_unique<TCPSocket>(client_fd));
}
bool TCPSocket::connect(int retry_count) {
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);
    addr.sin_addr.s_addr = inet_addr(this->ip.c_str());
    int result = 1;
    for (int i = 0; i <= retry_count && result != 0; i++) {
        result = ::connect(this->socket_num, (struct sockaddr*)&addr, sizeof(addr));
        if (result != 0) {
            std::cerr << "TCP connect failed: " << strerror(errno) << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        break;
    }
    return result == 0;
}
bool Socket::close() { 
    int result = ::close(this->socket_num);
    return result == 0;
}
bool TCPSocket::send(void* data, uint32_t size) { 
    int result = ::send(this->socket_num, data, size, 0);
    return result != -1;
}
std::vector<uint8_t> TCPSocket::recv(bool block) { 
    int result = ::recv(this->socket_num, this->recv_buffer, 1024, (block) ? MSG_WAITALL : 0);
    if (result == -1) {
        return {};
    }
    std::vector<uint8_t> out;
    for (size_t i = 0; i < (size_t)result; i++) {
        out.push_back(recv_buffer[i]);
    }
    return out;
}

TLSSocket::TLSSocket(int fd, SSL_CTX* ctx) : TCPSocket(fd) {
    this->type = SOCKET_TYPE::TLS;
    this->ctx = ctx;
    this->ssl = SSL_new(ctx);
    SSL_set_fd(this->ssl, this->socket_num);
}
std::unique_ptr<TLSSocket> TLSSocket::accept() {
    int client_fd = ::accept(this->socket_num, NULL, NULL);
    if (client_fd == -1) {
        return nullptr;
    }
    std::unique_ptr<TLSSocket> client = std::make_unique<TLSSocket>(client_fd, this->ctx);
    int result = SSL_accept(client->ssl);
    if (result != 1) {
        std::cerr << "SSL_accept failed (error " << SSL_get_error(client->ssl, result) << ")\n";
        ERR_print_errors_fp(stderr);
        return nullptr;
    }
    return client;
}
bool TLSSocket::connect(int retry_count) {
    if (!TCPSocket::connect(retry_count)) return false;
    int value = SSL_connect(this->ssl);
    if (value != 1) {
        std::cerr << "SSL_connect failed (error " << SSL_get_error(this->ssl, value) << ")\n";
        ERR_print_errors_fp(stderr);
        return false;
    }
    return true;
}
bool TLSSocket::close() {
    if (this->ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        this->ssl = nullptr;
    }
    return Socket::close();
}
bool TLSSocket::send(void* data, uint32_t size) {
    int result = SSL_write(this->ssl, data, size);
    return result > 0;
}
std::vector<uint8_t> TLSSocket::recv(bool block) {
    int result = SSL_read(this->ssl, this->recv_buffer, 1024);
    if (result <= 0) {
        return {};
    }
    std::vector<uint8_t> buffer;
    for (size_t i = 0; i < (size_t)result; i++) {
        buffer.push_back(recv_buffer[i]);
    }
    return buffer;
}