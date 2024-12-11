// #ifndef CLIENTWIN_H
// #define CLIENTWIN_H


// #include <Windows.h>
// #include <winsock2.h>
// #include <ws2tcpip.h>
// #include <iostream>
// #include <string>

// #pragma comment(lib, "Ws2_32.lib")

// class TCPClient {
// private:
//     SOCKET clientSocket;
//     sockaddr_in serverAddress;
//     int timeoutSec;

// public:
//     TCPClient(const std::string& serverIP, int port, int timeoutSec = 5);
//     ~TCPClient();

//     bool connectToServer();
//     bool sendMessage(char *buffer, int len);
//     int readMessage(char* buffer, size_t bufferSize);
//     void myClose();
// };

// #endif // CLIENTWIN_H
