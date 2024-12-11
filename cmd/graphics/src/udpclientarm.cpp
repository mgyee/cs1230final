// #include "udpclientarm.h"
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <errno.h>
// #include <cstring>
// #include <stdexcept>
// #include <iostream>

// UDPClient::UDPClient(const std::string& serverIP, int port, int timeoutSec) : timeoutSec(timeoutSec) {
//     // Create UDP socket
//     clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//     if (clientSocket < 0) {
//         std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
//         throw std::runtime_error("Socket creation failed");
//     }

//     // Prepare server address structure
//     memset(&serverAddress, 0, sizeof(serverAddress));
//     serverAddress.sin_family = AF_INET;
//     serverAddress.sin_port = htons(port);

//     // Convert IP address
//     if (inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr) <= 0) {
//         close(clientSocket);
//         std::cerr << "Invalid IP address\n";
//         throw std::runtime_error("Invalid IP address");
//     }

//     // Set socket timeout
//     struct timeval timeout;
//     timeout.tv_sec = timeoutSec;
//     timeout.tv_usec = 0;

//     // Set receive timeout
//     if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
//         close(clientSocket);
//         std::cerr << "Error setting receive timeout: " << strerror(errno) << std::endl;
//         throw std::runtime_error("Failed to set receive timeout");
//     }

//     // Enable address reuse
//     int optval = 1;
//     if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
//         close(clientSocket);
//         std::cerr << "Error setting address reuse: " << strerror(errno) << std::endl;
//         throw std::runtime_error("Failed to set address reuse");
//     }
// }

// // Send message method
// bool UDPClient::sendMessage(const char* buffer, int len) {
//     socklen_t serverAddrLen = sizeof(serverAddress);
//     int bytesSent = sendto(clientSocket, buffer, len, 0,
//                            reinterpret_cast<struct sockaddr*>(&serverAddress), serverAddrLen);

//     if (bytesSent < 0) {
//         std::cerr << "Send failed: " << strerror(errno) << std::endl;
//         return false;
//     }
//     return true;
// }

// // Read message method
// int UDPClient::readMessage(char* buffer, size_t bufferSize) {
//     sockaddr_in senderAddr;
//     socklen_t senderAddrSize = sizeof(senderAddr);

//     // Receive response
//     int bytesReceived = recvfrom(clientSocket, buffer,
//                                  bufferSize - 1, 0,
//                                  reinterpret_cast<struct sockaddr*>(&senderAddr), &senderAddrSize);

//     if (bytesReceived > 0) {
//         buffer[bytesReceived] = '\0';  // Null-terminate the received data
//         return bytesReceived;
//     }
//     else if (bytesReceived == 0) {
//         std::cerr << "No data received\n";
//         return -1;
//     }
//     else {
//         switch (errno) {
//         case ETIMEDOUT:
//             std::cerr << "Receive timeout\n";
//             break;
//         case ECONNRESET:
//             std::cerr << "Connection reset by peer (Port unreachable or network error)\n";
//             break;
//         default:
//             std::cerr << "Receive error: " << strerror(errno) << std::endl;
//         }
//         return 0;
//     }
// }

// // Close socket method
// void UDPClient::myClose() {
//     if (clientSocket >= 0) {
//         close(clientSocket);
//     }
// }

// // Destructor
// UDPClient::~UDPClient() {
//     if (clientSocket >= 0) {
//         close(clientSocket);
//     }
// }
