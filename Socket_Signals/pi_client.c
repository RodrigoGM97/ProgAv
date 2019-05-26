//Rodrigo Garcia Mayo
//A01024595
//Assignment 7 - Sockets and signals

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
// Custom libraries
#include "sockets.h"
#include "fatal_error.h"
//Poll libraries
#include <signal.h>
#include <poll.h>
#include <errno.h>

#define BUFFER_SIZE 1024

//Variable to monitor interrupt status
int disconnect = 0;

///// FUNCTION DECLARATIONS
void usage(char * program);
int openSocket(char * address, char * port);
void requestPI(int connection_fd);

//Functions for the signals
void setupHandlers();
void onInterrupt(int signal);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    setupHandlers();
    int connection_fd;
    
    printf("\n=== CLIENT FOR COMPUTING THE VALUE OF pi ===\n");

    //Check if the arguments were correct
    if (argc != 3)
    {
        usage(argv[0]);
    }

    //Start connection with server
    connection_fd = openSocket(argv[1], argv[2]);
    requestPI(connection_fd);

    // Close the socket
    close(connection_fd);
    
    return 0;
}

//How to run program
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {server_address} {port_number}\n", program);
    exit(EXIT_FAILURE);
}

//Connection setup
int openSocket(char * address, char * port)
{
    struct addrinfo hints;
    struct addrinfo * server_info = NULL;
    int connection_fd;
    
    //Addrinfo setup
    bzero(&hints, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(address, port, &hints, &server_info) == -1)
    {
        fatalError("getaddrinfo");
    }
    
    //Open the connection
    connection_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (connection_fd == -1)
    {
        close(connection_fd);
        fatalError("socket");
    }

    // Connect to the server
    if (connect(connection_fd, server_info->ai_addr, server_info->ai_addrlen) == -1)
    {
        fatalError("connect");
    }
    
    // Free the address info
    freeaddrinfo(server_info);
    
    return connection_fd;
}

//Communication between server and client to get PI
void requestPI(int connection_fd){
    char buffer[BUFFER_SIZE];
    int chars_read;

    double result; //Will store the PI number
    unsigned long int iterations; //Wanted iterations
    int final_iterations = 0; //Completed iterations
    
    //Poll
    struct pollfd test_fds;
    int poll_result;
    int timeout = 10;
    
    //Ask user for iterations
    printf("Enter the number of iterations you want for pi: ");
    scanf("%lu", &iterations);
    sprintf(buffer, "%lu", iterations);
    
    // Send the number of iterations to the server
    if (send(connection_fd, buffer, strlen(buffer)+1, 0) == -1){
        fatalError("send");
    }
    //Poll setup
    test_fds.fd = connection_fd;
    test_fds.events = POLLIN;

    //While client-server communication is still going
    while (!disconnect)
    {
        poll_result = poll(&test_fds, 1, timeout);
        //Poll waiting
        if (poll_result == 0)
        {
            //Continue
        }
        //Received message
        else if (poll_result > 0){
            if (test_fds.revents & POLLIN){
                break;
            }
        }
        else{
            // Poll error
            if (errno == EINTR)
            {
                if (send(connection_fd, "q", 1 + 1, 0) == -1){
                    fatalError("send");
                }
            }
            else
            {
                perror("ERROR: poll");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    // Clear the buffer
    bzero(buffer, BUFFER_SIZE);
    //Receive PI from server
    chars_read = recv(connection_fd, buffer, BUFFER_SIZE, 0);
    if (chars_read == -1){
        fatalError("recv");
    }
    sscanf(buffer, "%lf\n%d", &result, &final_iterations);
    //If program completed without interruption
    if(final_iterations == iterations){
        printf("The value for PI is: %.20lf\nNumber of iterations: %d\n", result, final_iterations);
        
    }
    //Program finished with interruption
    else{
        printf("Connection lost: \nThe last calculated value for PI was: %.20lf \nNumber of iterations: %d\n", result, final_iterations);
    }
}


//SIGINT handler
void setupHandlers(){
    struct sigaction new_action;
    struct sigaction old_action;
    
    // Prepare the structure to handle a signal
    new_action.sa_handler = onInterrupt;
    new_action.sa_flags = 0;
    sigfillset(&new_action.sa_mask);
    
    // Catch the signal for Ctrl-C
    sigaction(SIGINT, &new_action, &old_action);
}

//CTRL-C was pressed
void onInterrupt(int signal){
    printf("\n");
    disconnect = 1;
}
