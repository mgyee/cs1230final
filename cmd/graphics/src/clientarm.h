#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <stdexcept>

class TCPClient {
private:
    int clientSocket;
    struct sockaddr_in serverAddress;

public:
    // Constructor to initialize the socket and server address
    TCPClient(const std::string& serverIP, int port);

    // Method to connect to the server and send a message
    bool connectToServer();
    bool sendMessage(const std::string& message);
    int readMessage(char *buffer);
    void myClose();

    // Destructor to clean up socket resources
    ~TCPClient();
};

#endif // CLIENTSOCKET_H
