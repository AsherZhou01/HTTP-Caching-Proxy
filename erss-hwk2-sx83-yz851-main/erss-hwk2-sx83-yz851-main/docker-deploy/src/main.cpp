#include "ProxyServer.hpp"
#include "Logger.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <system_error>
#include "main.hpp"
using namespace std;

#define MAX_SIZE 65536

int main()
{

    try
    {
        file.open("myLog");
    }
    catch (const exception &e)
    {
        throw std::runtime_error("Failed to open file");
    }

    DataCache cache;                 // Initialize the cache.
    const char *proxyPort = "12345"; // Define the port the proxy server will listen on.

    struct addrinfo hints, *res;
    int serverSocket, yes = 1;

    // Setup server socket
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, proxyPort, &hints, &res) != 0)
    {
        std::cerr << "[error] Can't resolve address info for proxy." << std::endl;
        exit(EXIT_FAILURE);
    }

    serverSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverSocket < 0)
    {
        std::cerr << "[error] Can't create socket." << std::endl;
        exit(EXIT_FAILURE);
    }

    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (::bind(serverSocket, res->ai_addr, res->ai_addrlen) < 0)
    {
        std::cerr << "[error] Can't bind socket." << std::endl;
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 100) < 0)
    {
        std::cerr << "[error] Can't listen on socket." << std::endl;
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res); // Free the allocated address information
    std::cout << "Proxy server ready for connections on port " << proxyPort << std::endl;

    // Daemonize the process
    if (daemon(1, 0) == -1)
    { // (1,0) to not change the current directory and not close standard file descriptors.
        std::cerr << "Failed to daemonize the process." << std::endl;
        exit(EXIT_FAILURE);
    }
    int ID=0;

    // Main loop to wait for and handle connections.
    while (true)
    {
        ConnectionDetails *connectionData = new ConnectionDetails(); // Allocate new struct to hold connection data.

        // Wait for connection
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof client_addr;
        int clientSocket = accept(serverSocket, (struct sockaddr *)&client_addr, &addr_size);
        if (clientSocket < 0)
        {
            std::cerr << "[error] Failed to accept connection." << std::endl;
            continue; // Skip this iteration and continue listening for new connections.
        }

        connectionData->connectionFD = clientSocket;                                          // Store the client socket.
        connectionData->clientIP = inet_ntoa(((struct sockaddr_in *)&client_addr)->sin_addr); // Store client IP.
        cout << "connected with " << connectionData->clientIP << endl;
        
        connectionData->connectionID=ID++;
        connectionData->dataCache=&cache;
        pthread_t thread;
        pthread_create(&thread, NULL, manageClientRequest, (void *)connectionData); // Create a new thread to handle the request.
        pthread_detach(thread);                                                     // Optionally detach the thread.
    }

    close(serverSocket); // This line will likely never be reached.
    return 0;
}
