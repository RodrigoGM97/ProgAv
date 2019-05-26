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
// Custom libraries
#include "sockets.h"
#include "fatal_error.h"
//Poll libraries
#include <signal.h>
#include <poll.h>
#include <errno.h>

//Global variables
int disconnect = 0;

#define BUFFER_SIZE 1024
#define MAX_QUEUE 5

///// FUNCTION DECLARATIONS
void usage(char * program);
void waitForConnections(int server_fd);
void attendRequest(int client_fd);

void setupHandlers();
void onInterrupt(int signal);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    setupHandlers();
    int server_fd;

    printf("\n=== SERVER FOR COMPUTING THE VALUE OF pi ===\n");

    if (argc != 2) //Check if the arguments were correct
    {
        usage(argv[0]);
    }

	// Show the IPs assigned to this computer
	printLocalIPs();

    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
    
	// Listen for connections from the clients
    waitForConnections(server_fd);
    // Close the socket
    close(server_fd);

    return 0;
}

///// FUNCTION DEFINITIONS

/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char * program)
{
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}


/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pid_t new_pid;

    // Get the size of the structure to store client information
    client_address_size = sizeof client_address;

    while (1)
    {
        // ACCEPT
        // Wait for a client connection
        client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
        if (client_fd == -1)
        {
            fatalError("accept");
        }
         
        // Get the data from the client
        inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
        printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);

        // FORK
        // Create a new child process to deal with the client
        new_pid = fork();
        // Parent
        if (new_pid > 0)
        {
            // Close the new socket
            close(client_fd);
        }
        else if (new_pid == 0)
        {
            // Close the main server socket to avoid interfering with the parent
            close(server_fd);
            printf("Child process %d dealing with client\n", getpid());
            // Deal with the client
            attendRequest(client_fd);
            // Close the new socket
            close(client_fd);
            // Terminate the child process
            exit(EXIT_SUCCESS);
        }
        else
        {
            fatalError("fork");
        }

    }
}

/*
    Hear the request from the client and send an answer
*/
void attendRequest(int client_fd)
{
    char buffer[BUFFER_SIZE];
    int chars_read;
    unsigned long int current_it = 0; //Current iterations
    unsigned long int iterations; //Wanted iterations
    //Poll
    struct pollfd test_fds;
    int poll_result;
    int timeout = 10;
    //GetPi variables
    double pi_result = 4;
    int sign = -1;
    unsigned long int divisor = 3;
    
    // Reset buffer
    bzero(buffer, BUFFER_SIZE);
    
    // Gets the number of iterations
    chars_read = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if (chars_read == -1){
        fatalError("recv");
    }
    sscanf(buffer, "%lu", &iterations);
    printf(" > Got request from client with iterations=%lu\n", iterations);
    
    //Poll setup
    test_fds.fd = client_fd;
    test_fds.events = POLLIN;
    poll_result = poll(&test_fds, 1, timeout);
    
    //Compute PI using poll
    while (current_it < iterations){
        if(current_it % 1000 == 0){
            poll_result = poll(&test_fds, 1, timeout);
        }
        if (poll_result == 0){
            pi_result += sign * (4.0/divisor);
            sign *= -1;
            divisor += 2;
            current_it++;
        }
        // Server received message
        else if (poll_result > 0){
            if (test_fds.revents & POLLIN){
                sprintf(buffer, "%.20lf\n%lu", pi_result, current_it);
                send(client_fd, buffer, strlen(buffer) + 1, 0);
                break;
            }
        }
        //Poll error
        else{
            if (errno == EINTR)
            {
                break;
            }
        }
    }
    //Case: Incomplete iterations
    if(current_it < iterations)
    {
        printf("\n > Connection lost!\n > Sending incomplete PI approximation =%.20lf\n > Number of iterations completed: %lu\n", pi_result, current_it);
        sprintf(buffer, "%.20lf\n%lu", pi_result, current_it);
    }
    //Case: Completed iterations
    else
    {
        printf("\n > Sending PI approximation to client...\n > The calculated value for PI = %.20lf\n > Number of iterations: %lu\n", pi_result, current_it);
        sprintf(buffer, "%.20lf\n%lu", pi_result, current_it);
    }
    //Send PI approximation to client
    send(client_fd, buffer, strlen(buffer) + 1, 0);
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
    disconnect = 1;
}

