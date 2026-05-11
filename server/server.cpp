#include <iostream>
#include "../shared/socket.hpp"

int main() {
    TCPSocket server("", 9001);
    if (!server.bind()) {
        std::cout << "Failed to bind socket\n";
        return 1;
    }
    server.listen(10);
    std::cout << "Server listening on port 9001\n";

    while (true) {
        std::unique_ptr<TCPSocket> client = server.accept();
        if (!client) continue;
        std::cout << "Client connected\n";
        auto data = client->recv(true);
        std::string msg(data.begin(), data.end());
        std::cout << msg << "\n";
        client->close();
    }

    return 0;
}