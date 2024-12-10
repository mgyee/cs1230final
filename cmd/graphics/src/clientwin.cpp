#include "clientwin.h"

TCPClient::TCPClient(const std::string& serverIP, int port) {
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
        std::cerr << "Error creating socket\n";
        throw std::runtime_error("Socket creation failed");
    }

    // Prepare server address structure
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr);
}


void TCPClient::connectAndSend(const std::string& message) {
    // Connect to server
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        WSACleanup();
        std::cerr << "Connection failed\n";
        return;
    }

    // Send message
    send(clientSocket, message.c_str(), message.length(), 0);

    // Receive response
    char buffer[1024] = {0};
    recv(clientSocket, buffer, sizeof(buffer), 0);
    std::cout << "Received: " << buffer << std::endl;

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();
}

TCPClient::~TCPClient() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        WSACleanup();
    }
}


