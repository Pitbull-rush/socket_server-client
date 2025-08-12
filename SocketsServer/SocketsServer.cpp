#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

int main() {
    WSADATA wsaData{};
    addrinfo hints{}, * addrResult = nullptr;
    SOCKET listenSocket = INVALID_SOCKET;

    const int RECV_BUF_SIZE = 512;
    char recvBuffer[RECV_BUF_SIZE + 1];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM;  
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;     

    if (getaddrinfo(NULL, "7777", &hints, &addrResult) != 0) {
        std::cerr << "getaddrinfo failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    listenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "socket failed: " << WSAGetLastError() << "\n";
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    if (bind(listenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << "\n";
        freeaddrinfo(addrResult);
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(addrResult);

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << "\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port 7777...\n";

    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "accept failed: " << WSAGetLastError() << "\n";
            continue;
        }

        std::cout << "Client connected.\n";

        while (true) {
            ZeroMemory(recvBuffer, sizeof(recvBuffer));
            int bytesReceived = recv(clientSocket, recvBuffer, RECV_BUF_SIZE, 0);

            if (bytesReceived > 0) {
                if (bytesReceived > RECV_BUF_SIZE) bytesReceived = RECV_BUF_SIZE;
                recvBuffer[bytesReceived] = '\0';

                std::string received = recvBuffer;
                std::cout << "Received (" << bytesReceived << "): " << received << "\n";

                if (received == "exit") {
                    std::cout << "Client requested disconnect (exit).\n";
                    break;
                }

                std::string reply = "Server: " + received;
                int bytesSent = send(clientSocket, reply.c_str(), (int)reply.size(), 0);
                if (bytesSent == SOCKET_ERROR) {
                    std::cerr << "send failed: " << WSAGetLastError() << "\n";
                    break;
                }
            }
            else if (bytesReceived == 0) {
                std::cout << "Client disconnected gracefully.\n";
                break;
            }
            else { 
                std::cerr << "recv failed: " << WSAGetLastError() << "\n";
                break;
            }
        } 

        closesocket(clientSocket);
        std::cout << "Client socket closed. Waiting for new connections...\n";
    } 
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
