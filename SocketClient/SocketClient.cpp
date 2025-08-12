#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    ADDRINFO hints{};
    ADDRINFO* addrResult = nullptr;

    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_protocol = IPPROTO_TCP; 

    const char* serverAddress = "127.0.0.1"; 
    const char* serverPort = "7777";         

    if (getaddrinfo(serverAddress, serverPort, &hints, &addrResult) != 0) {
        std::cerr << "getaddrinfo failed\n";
        WSACleanup();
        return 1;
    }

    SOCKET ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    if (connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "Unable to connect to server\n";
        closesocket(ConnectSocket);
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(addrResult);

    std::string sendBuffer;
    char recvBuffer[512];

    while (true) {
        std::cout << "Enter message (exit to quit): ";
        std::getline(std::cin, sendBuffer);

        if (sendBuffer == "exit")
            break;

        int bytesSent = send(ConnectSocket, sendBuffer.c_str(), (int)sendBuffer.size(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Send failed\n";
            break;
        }

        int bytesRecv = recv(ConnectSocket, recvBuffer, sizeof(recvBuffer) - 1, 0);
        if (bytesRecv > 0) {
            recvBuffer[bytesRecv] = '\0';
            std::cout << "Server response: " << recvBuffer << "\n";
        }
        else if (bytesRecv == 0) {
            std::cout << "Connection closed by server\n";
            break;
        }
        else {
            std::cerr << "Recv failed\n";
            break;
        }
    }

    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}
