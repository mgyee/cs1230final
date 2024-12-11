// #include "udpclientwin.h"

// UDPClient::UDPClient(const std::string& serverIP, int port, int timeoutSec) : timeoutSec(timeoutSec) {
//     // Initialize Winsock
//     WSADATA wsaData;
//     if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//         std::cerr << "WSAStartup failed\n";
//         throw std::runtime_error("WSAStartup failed");
//     }

//     // Create UDP socket
//     clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//     if (clientSocket == INVALID_SOCKET) {
//         WSACleanup();
//         std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
//         throw std::runtime_error("Socket creation failed");
//     }

//     // Prepare server address structure
//     ZeroMemory(&serverAddress, sizeof(serverAddress));
//     serverAddress.sin_family = AF_INET;
//     serverAddress.sin_port = htons(port);

//     // Convert IP address
//     if (inet_pton(AF_INET, serverIP.c_str(), &serverAddress.sin_addr) <= 0) {
//         closesocket(clientSocket);
//         WSACleanup();
//         std::cerr << "Invalid IP address\n";
//         throw std::runtime_error("Invalid IP address");
//     }

//     // Set socket timeout
//     DWORD timeout = timeoutSec * 1000; // Convert to milliseconds
//     setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
//     int optval = 1;
//     setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
// }

// // Send message method
// bool UDPClient::sendMessage(const char* buffer, int len) {
//     int bytesSent = sendto(clientSocket, buffer, len, 0,
//                            reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress));

//     if (bytesSent == SOCKET_ERROR) {
//         std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
//         return false;
//     }
//     return true;
// }

// // Read message method
// int UDPClient::readMessage(char* buffer, size_t bufferSize) {
//     sockaddr_in senderAddr;
//     int senderAddrSize = sizeof(senderAddr);

//     // Receive response
//     int bytesReceived = recvfrom(clientSocket, buffer,
//                                  static_cast<int>(bufferSize) - 1, 0,
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
//         int error = WSAGetLastError();
//         switch (error) {
//         case WSAETIMEDOUT:
//             std::cerr << "Receive timeout\n";
//             break;
//         case WSAECONNRESET:
//             std::cerr << "Connection reset by peer (Port unreachable or network error)\n";
//             break;
//         default:
//             std::cerr << "Receive error: " << error << std::endl;
//         }
//         return 0;
//     }
// }

// // Close socket method
// void UDPClient::myClose() {
//     if (clientSocket != INVALID_SOCKET) {
//         closesocket(clientSocket);
//     }
// }

// // Destructor
// UDPClient::~UDPClient() {
//     if (clientSocket != INVALID_SOCKET) {
//         closesocket(clientSocket);
//         WSACleanup();
//     }
// }
