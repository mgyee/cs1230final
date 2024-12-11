#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

class UDPClient {
private:
    int clientSocket;  // Changed from SOCKET to int for POSIX compatibility
    sockaddr_in serverAddress;
    int timeoutSec;

public:
    // Constructor - takes server IP, port, and timeout in seconds
    UDPClient(const std::string& serverIP, int port, int timeoutSec);

    // Send a message to the server
    // Returns true if send is successful, false otherwise
    bool sendMessage(const char* buffer, int len);

    // Read a message from the server
    // Returns number of bytes received, 0 on error, -1 on no data
    int readMessage(char* buffer, size_t bufferSize);

    // Close the socket
    void myClose();

    // Destructor
    ~UDPClient();
};

#endif // UDPCLIENT_H