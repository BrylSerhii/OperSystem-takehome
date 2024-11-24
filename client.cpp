#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

void communicate_with_server(SOCKET server_socket) {
    while (true) {
        std::cout << "Enter a command ('status' or an integer for computation, 'exit' to quit): ";
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit") break;

        send(server_socket, input.c_str(), input.size(), 0);

        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            std::cout << "Server response: " << buffer << std::endl;
        } else {
            std::cerr << "Connection closed by server or error occurred\n";
            break;
        }
    }
}

int main() {
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server\n";
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to the server.\n";
    communicate_with_server(client_socket);

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
