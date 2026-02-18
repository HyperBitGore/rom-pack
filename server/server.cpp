#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


int main () {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9001);
    
    int val = bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (val != 0) {
        std::cout << "Failed to bind socket, error: " << errno << "\n";
        return 1; 
    }
    listen(server_fd, 10);
    
    std::cout << "Server listening on port 9001" << std::endl;
    
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        std::cout << "Client connected" << std::endl;
        
        close(client_fd);
    }
    
    return 0;
}