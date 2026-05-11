cmake_minimum_required(VERSION 3.10)
project(ServerClient VERSION 1.0)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

# Server executable
add_executable(server
    server/server.cpp
    shared/socket.cpp
)
target_link_libraries(server OpenSSL::SSL OpenSSL::Crypto)

# Client executable
add_executable(client
    client/client.cpp
    shared/socket.cpp
)
target_link_libraries(client OpenSSL::SSL OpenSSL::Crypto)