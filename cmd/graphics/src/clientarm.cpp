#include "clientarm.h"
#include <iostream>
#include <cstring>
#include <stdexcept>

TCPClient::TCPClient(const std::string& serverIP, int port) {
    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error creating socket\n";
        throw std::runtime_error("Socket creation failed");
    }

    // Prepare server address structure
    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    // Convert IP address
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr) <= 0) {
        close(clientSocket);
        std::cerr << "Invalid IP address\n";
        throw std::runtime_error("Invalid IP address");
    }
}

bool TCPClient::connectToServer() {
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Connection failed\n";
        close(clientSocket);
        return false;
    }
    return true;
}

bool TCPClient::sendMessage(const std::string& message) {
    // Send message
    ssize_t bytesSent = send(clientSocket, message.c_str(), message.length(), 0);
    if (bytesSent < 0) {
        std::cerr << "Send failed\n";
        return false;
    }

    return true;
}

int TCPClient::readMessage(char *buffer) {
    // Receive response
    struct timeval timeout;
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Failed to set socket timeout\n";
        return -1;
    }


    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the received data
        std::cout << "Received: " << buffer << std::endl;

    } else if (bytesReceived == 0) {
        std::cerr << "Receive no bytes lol\n";
        // error kill yourself
        return -1;
    } else {
        // Need to predict
        std::cerr << "Connection closed or timeout\n";
        return 1;
    }

    // use worldstate
    return 0;
}

void TCPClient::myClose() {
    close(clientSocket);
}

TCPClient::~TCPClient() {
    if (clientSocket >= 0) {
        close(clientSocket);
    }
}
