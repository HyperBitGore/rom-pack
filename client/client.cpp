#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int main() {
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9001);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    int result = connect(client_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (result != 0) {
        std::cout << "Error connecting to server, error: " << errno << "\n";
    }

    std::cout << "Connected to server" << std::endl;
    
    close(client_fd);
    
    return 0;
}