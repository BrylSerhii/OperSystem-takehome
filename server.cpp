#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

std::mutex status_mutex;
bool is_computing = false;
int computations_done = 0;

double function_g(int x) {
    return x * x + 0.5; // Просте обчислення
}

void handle_client(SOCKET client_socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) break;

        std::string command(buffer);
        if (command == "status") {
            std::lock_guard<std::mutex> lock(status_mutex);
            std::string status = "Computing: " + std::to_string(is_computing) +
                                 "\nComputations Done: " + std::to_string(computations_done) + "\n";
            send(client_socket, status.c_str(), status.size(), 0);
        } else {
            int x = std::stoi(command);
            {
                std::lock_guard<std::mutex> lock(status_mutex);
                is_computing = true;
            }

            double result = function_g(x);

            {
                std::lock_guard<std::mutex> lock(status_mutex);
                is_computing = false;
                computations_done++;
            }

            std::string response = "Result: " + std::to_string(result) + "\n";
            send(client_socket, response.c_str(), response.size(), 0);
        }
    }
    closesocket(client_socket);
}

int main() {
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is running on port 8080...\n";

    while (true) {
        sockaddr_in client_addr{};
        int client_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Failed to accept connection\n";
            continue;
        }

        std::thread(handle_client, client_socket).detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
