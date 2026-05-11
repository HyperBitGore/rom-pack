#include <iostream>
#include "../shared/socket.hpp"

int main() {
    TCPSocket client("127.0.0.1", 9001);
    if (!client.connect()) {
        std::cout << "Error connecting to server\n";
        return 1;
    }
    std::cout << "Connected to server\n";
    const char* msg = "Hello server";
    client.send((void*)msg, sizeof("Hello server"));
    client.close();

    return 0;
}