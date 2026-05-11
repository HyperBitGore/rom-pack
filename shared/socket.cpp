#include "socket.hpp"
#include <arpa/inet.h>
#include <memory>
#include <netinet/in.h>
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
bool TCPSocket::connect() {
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);
    addr.sin_addr.s_addr = inet_addr(this->ip.c_str());

    int result = ::connect(this->socket_num, (struct sockaddr*)&addr, sizeof(addr));
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