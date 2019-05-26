//Rodrigo Garcia Mayo
//A01024595
//Assignment 5 - Network Blackjack Game

#include "sockets.h"


int prepareServer(char * port, int max_queue)
{
    struct addrinfo hints;
    struct addrinfo * address_info = NULL;
    int server_fd;
    
    // Clear the structure before filling the data
    bzero(&hints, sizeof hints);
    
    // Configure the structure to search for addresses
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    // Get the list of addresses for the server machine
    if (getaddrinfo(NULL, port, &hints, &address_info) == -1)
    {
        perror("ERROR: getaddrinfo");
        exit(EXIT_FAILURE);
    }
    
    // Open the socket
    server_fd = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if (server_fd == -1)
    {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
    }
    
    int yes = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int)) == -1)
    {
        close(server_fd);
        freeaddrinfo(address_info);
        perror("ERROR: setsockopt");
        exit(EXIT_FAILURE);
    }
    
    // Bind the socket to an address and port
    if (bind(server_fd, address_info->ai_addr, address_info->ai_addrlen) == -1)
    {
        close(server_fd);
        freeaddrinfo(address_info);
        perror("ERROR: bind");
        exit(EXIT_FAILURE);
    }
    
    // Free the list of addresses
    freeaddrinfo(address_info);
    
    // Configure the socket to listen for incoming connections
    if (listen(server_fd, max_queue) == -1)
    {
        close(server_fd);
        perror("ERROR: listen");
        exit(EXIT_FAILURE);
    }
    
    return server_fd;
}

int connectToServer(char * address, char * port)
{
    struct addrinfo hints;
    struct addrinfo * address_info = NULL;
    int server_fd;
    
    // Clear the structure before filling the data
    bzero(&hints, sizeof hints);
    
    // Configure the structure to search for addresses
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    // Get the list of addresses for the server machine
    if (getaddrinfo(address, port, &hints, &address_info) == -1)
    {
        perror("ERROR: getaddrinfo");
        exit(EXIT_FAILURE);
    }
    
    // Open the socket
    server_fd = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if (server_fd == -1)
    {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
    }
    
    // Connect to the server
    if (connect(server_fd, address_info->ai_addr, address_info->ai_addrlen) == -1)
    {
        close(server_fd);
        freeaddrinfo(address_info);
        perror("ERROR: bind");
        exit(EXIT_FAILURE);
    }
    
    // Free the list of addresses
    freeaddrinfo(address_info);
    
    return server_fd;
}

int recvMessage(int connection_fd, char * buffer, int buffer_size)
{
    int chars_read;

    // Read the request
    bzero(buffer, buffer_size);
    
    chars_read = recv(connection_fd, buffer, buffer_size, 0);
    // An error in the communication
    if (chars_read == -1)
    {
        perror("ERROR: recv");
    }
    // Connection closed by the client
    if (chars_read == 0)
    {
        printf("Client has disconnected\n");
        exit(0);
    }

    return chars_read;
} 
