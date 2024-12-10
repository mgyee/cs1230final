#include "clientwin.h"
// #include <iostream>
// #include <cstring>
// #include <stdexcept>
// #include <WinSock2.h>
// #include <ws2tcpip.h>
// #pragma comment(lib, "Ws2_32.lib")

TCPClient::TCPClient(const std::string& serverIP, int port, int timeoutSec) : timeoutSec(timeoutSec) {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        throw std::runtime_error("WSAStartup failed");
    }

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        WSACleanup();
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        throw std::runtime_error("Socket creation failed");
    }

    // Prepare server address structure
    ZeroMemory(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    // Convert IP address
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr) <= 0) {
        closesocket(clientSocket);
        WSACleanup();
        std::cerr << "Invalid IP address\n";
        throw std::runtime_error("Invalid IP address");
    }

    // Set socket timeout
    DWORD timeout = timeoutSec * 1000; // Convert to milliseconds
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
}

bool TCPClient::connectToServer() {
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        return false;
    }
    return true;
}

bool TCPClient::sendMessage(char *buffer, int len) {
    int bytesSent = send(clientSocket, buffer, len, 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

int TCPClient::readMessage(char* buffer, size_t bufferSize) {
    // Receive response
    int bytesReceived = recv(clientSocket, buffer, static_cast<int>(bufferSize) - 1, 0);

    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the received data
        //std::cout << "Received: " << buffer << std::endl;
        return bytesReceived;
    }
    else if (bytesReceived == 0) {
        std::cerr << "Connection closed by server\n";
        return -1;
    }
    else {
        int error = WSAGetLastError();
        if (error == WSAETIMEDOUT) {
            std::cerr << "Receive timeout\n";
        }
        else {
            std::cerr << "Receive error: " << error << std::endl;
        }
        return 0;
    }
}

void TCPClient::myClose() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
    }
}

TCPClient::~TCPClient() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        WSACleanup();
    }
}


