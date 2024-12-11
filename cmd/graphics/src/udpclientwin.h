#ifndef UDPCLIENTWIN_H
#define UDPCLIENTWIN_H


#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>


class UDPClient {
private:
    SOCKET clientSocket;
    sockaddr_in serverAddress;
    int timeoutSec;

public:
    UDPClient(const std::string& serverIP, int port, int timeoutSec);

    bool sendMessage(const char* buffer, int len);
    int readMessage(char* buffer, size_t bufferSize);
    void myClose();

    ~UDPClient();
};

#endif // UDPCLIENTWIN_H
