//Rodrigo García Mayo
//A01024595
//08_Simple_Bank

/*
    Program for a simple bank server
    It uses sockets and threads
    The server will process simple transactions on a limited number of accounts

    Gilberto Echeverria
    gilecheverria@yahoo.com
    29/03/2018
    08/04/2018  Added initialization of account balances
    27/10/2018  Simplify the function 'processOperation' by validating the account before the switch
    14/03/2019  Added the new operation for transfer from one account to another
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>

// Custom libraries
#include "bank_codes.h"
#include "sockets.h"
#include "fatal_error.h"

#define MAX_ACCOUNTS 5
#define BUFFER_SIZE 1024
#define MAX_QUEUE 5

///// Structure definitions

// Data for a single bank account
typedef struct account_struct {
    int id;
    int pin;
    float balance;
} account_t;

// Data for the bank operations
typedef struct bank_struct {
    // Store the total number of operations performed
    int total_transactions;
    // An array of the accounts
    account_t * account_array;
} bank_t;

// Structure for the mutexes to keep the data consistent
typedef struct locks_struct {
    // Mutex for the number of transactions variable
    pthread_mutex_t transactions_mutex;
    // Mutex array for the operations on the accounts
    pthread_mutex_t * account_mutex;
} locks_t;

// Data that will be sent to each structure
typedef struct data_struct {
    // The file descriptor for the socket
    int connection_fd;
    // A pointer to a bank data structure
    bank_t * bank_data;
    // A pointer to a locks structure
    locks_t * data_locks;
} thread_data_t;

typedef struct message{
    int operation;
    int account_from;
    int account_to;
    float amount;
} msg_t;


// Check CTRL-C activation
int interrupt_exit = 0;

///// FUNCTION DECLARATIONS
void usage(char * program);
void setupHandlers();
void initBank(bank_t * bank_data, locks_t * data_locks);
void readBankFile(bank_t * bank_data);
void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks);
void * attentionThread(void * arg);
void closeBank(bank_t * bank_data, locks_t * data_locks);
int checkValidAccount(int account);
void writeBankFile(bank_t * bank_data);
/*
    TODO: Add your function declarations here

*/
void getMessage(msg_t* msg, char* buffer);
float check_Balance(thread_data_t* data, int account);
float deposit(thread_data_t* data, int account, float amount);
float withdraw(thread_data_t* data, int account, float amount);
void transfer(thread_data_t* data, int account_from, int account_to, float amount);
void onInterrupt(int signal);

///// MAIN FUNCTION
int main(int argc, char * argv[])
{
    int server_fd;
    bank_t bank_data;
    locks_t data_locks;

    printf("\n=== SIMPLE BANK SERVER ===\n");

    // Check the correct arguments
    if (argc != 2)
    {
        usage(argv[0]);
    }

    // Configure the handler to catch SIGINT
    setupHandlers();

    // Initialize the data structures
    initBank(&bank_data, &data_locks);

    // Show the IPs assigned to this computer
    printLocalIPs();
    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
    // Listen for connections from the clients
    waitForConnections(server_fd, &bank_data, &data_locks);
    // Close the socket
    close(server_fd);

    writeBankFile(&bank_data);

    // Clean the memory used
    closeBank(&bank_data, &data_locks);

    //Finish the main thread
    pthread_exit(NULL);

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
    Modify the signal handlers for specific events
*/
void setupHandlers()
{
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
void onInterrupt(int signal)
{
    printf("\nServer interrupted!\n");
    interrupt_exit = 1;
}


/*
    Function to initialize all the information necessary
    This will allocate memory for the accounts, and for the mutexes
*/
void initBank(bank_t * bank_data, locks_t * data_locks)
{
    // Set the number of transactions
    bank_data->total_transactions = 0;

    // Allocate the arrays in the structures
    bank_data->account_array = malloc(MAX_ACCOUNTS * sizeof (account_t));
    // Allocate the arrays for the mutexes
    data_locks->account_mutex = malloc(MAX_ACCOUNTS * sizeof (pthread_mutex_t));

    // Initialize the mutexes, using a different method for dynamically created ones
    //data_locks->transactions_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_init(&data_locks->transactions_mutex, NULL);
    for (int i=0; i<MAX_ACCOUNTS; i++)
    {
        //data_locks->account_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_init(&data_locks->account_mutex[i], NULL);
        // Initialize the account balances too
        bank_data->account_array[i].balance = 0.0;
    }

    // Read the data from the file
    readBankFile(bank_data);
}


/*
    Get the data from the file to initialize the accounts
*/
void readBankFile(bank_t * bank_data)
{
    FILE * file_ptr = NULL;
    char buffer[BUFFER_SIZE];
    int account = 0;
    char * filename = "accounts.txt";

    file_ptr = fopen(filename, "r");
    if (!file_ptr)
    {
        fatalError("ERROR: fopen");
    }

    // Ignore the first line with the headers
    fgets(buffer, BUFFER_SIZE, file_ptr);
    // Read the rest of the account data
    while( fgets(buffer, BUFFER_SIZE, file_ptr) )
    {
        sscanf(buffer, "%d %d %f", &bank_data->account_array[account].id, &bank_data->account_array[account].pin, &bank_data->account_array[account].balance);
        account++;
    }

    fclose(file_ptr);
}

//Update accounts file
void writeBankFile(bank_t * bank_data)
{
    FILE * file_ptr = NULL;
    char buffer[BUFFER_SIZE];
    int account = 0;
    char * filename = "accounts.txt";

    file_ptr = fopen(filename, "w");
    rewind(file_ptr);
    if (!file_ptr)
    {
        fatalError("ERROR: fopen");
    }

    int n;
    n = (sizeof(bank_data->account_array)/sizeof(int));
    //Headers
    sprintf(buffer, "Account_number PIN Balance\n");
    fwrite(buffer, strlen(buffer), 1, file_ptr);
    //Update all accounts
    while(account <= n)
    {
        sprintf(buffer, "%d %d %.2f", bank_data->account_array[account].id, bank_data->account_array[account].pin, bank_data->account_array[account].balance);
        fwrite(buffer, strlen(buffer), 1, file_ptr);
        sprintf(buffer, "\n");
        fwrite(buffer, strlen(buffer), 1, file_ptr);
        account++;
    }
    fclose(file_ptr);
}


/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd, bank_t * bank_data, locks_t * data_locks)
{
    struct sockaddr_in client_address;
    socklen_t client_address_size;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pthread_t new_tid;
    thread_data_t * connection_data = NULL;
    int poll_response;
    int timeout = 500;		// Time in milliseconds (0.5 seconds)
    int transactions = 0;

    // Get the size of the structure to store client information
    client_address_size = sizeof client_address;

    while (1)
    {
        // Create a structure array to hold the file descriptors to poll
        struct pollfd test_fds[1];
        // Fill in the structure
        test_fds[0].fd = server_fd;
        test_fds[0].events = POLLIN;    // Check for incomming data

        poll_response = poll(test_fds, 1, timeout);
        //Interruption (system call)
        if (poll_response == -1)
        {
            if (errno == EINTR)
            {
                break;
            }
        }
        //Check for interruptions
        if (poll_response == 0)
        {
            if (interrupt_exit)
                break;
        }
        //Continue bank operations
        if (poll_response > 0)
        {
            if (test_fds[0].revents & POLLIN)
            {
                // ACCEPT
                // Wait for a client connection
                client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
                if (client_fd == -1)
                {
                    fatalError("ERROR: accept");
                }

                // Get the data from the client
                inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
                printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);

                // Prepare the structure to send to the thread
                connection_data = malloc(sizeof(thread_data_t));
                connection_data->bank_data = bank_data;
                connection_data->connection_fd = client_fd;
                connection_data->data_locks = data_locks;
                transactions++;
                // CREATE A THREAD
                if (pthread_create(&new_tid, NULL, attentionThread, connection_data) == 0)
                    printf("Thread created\n");
            }
        }
    }

    // Show the number of total transactions
    if (transactions == 0)
        printf("Transactions completed: %d\n", transactions);
    else
        printf("Transactions completed: %d\n", connection_data->bank_data->total_transactions);
}

/*
    Hear the request from the client and send an answer
*/
void * attentionThread(void * arg)
{
    char buffer[BUFFER_SIZE];
    int poll_response;
    int timeout = 500;		// Time in milliseconds (0.5 seconds)
    // Receive the data for the bank, mutexes and socket file descriptor
    thread_data_t* data = arg;
    float balance;
    //Account verifications variable
    int check1;
    int check2;
    // Loop to listen for messages from the client
    while (!interrupt_exit)
    {
        // Create a structure array to hold the file descriptors to poll
        struct pollfd test_fds[1];
        // Fill in the structure
        test_fds[0].fd = data->connection_fd;
        test_fds[0].events = POLLIN;    // Check for incomming data
        poll_response = poll(test_fds, 1, timeout);

        if (poll_response == -1)
        {
            if (errno == EINTR)
            {
                break;
            }
        }
        // Receive the request
        else if (poll_response > 0)
        {
            if (recvString(data->connection_fd, buffer, BUFFER_SIZE) == 0)
            {
                printf("Client disconnected\n");
                break;
            }
            //Variable to store data from client
            msg_t income_msg;
            //Receive the message to store in buffer
            getMessage(&income_msg, buffer);
            //Determine operation
            switch (income_msg.operation)
            {
                case CHECK:
                    if (!checkValidAccount(income_msg.account_from)) {
                        sprintf(buffer, "%d %f", NO_ACCOUNT, balance);
                        sendString(data->connection_fd, buffer, BUFFER_SIZE);
                    } else {
                        printf("Checking current balance for account: %d\n", income_msg.account_from);
                        //Generate new balance
                        balance = check_Balance(data, income_msg.account_from);
                        printf("Current balance: %f\n", balance);
                        sprintf(buffer, "%d %f", OK, balance);
                        sendString(data->connection_fd, buffer, BUFFER_SIZE);
                    }
                    break;
                case DEPOSIT:
                    if (!checkValidAccount(income_msg.account_to)) {
                        sprintf(buffer, "%d %f", NO_ACCOUNT, balance);
                        sendString(data->connection_fd, buffer, BUFFER_SIZE);
                    }
                    else
                    {
                        printf("Making deposit in account: %d\n", income_msg.account_to);
                        printf("Original balance: %f\n", data->bank_data->account_array[income_msg.account_to].balance);
                        //Generate new balance
                        balance = deposit(data, income_msg.account_to, income_msg.amount);
                        printf("Account balance: %f\n", data->bank_data->account_array[income_msg.account_to].balance);
                        sprintf(buffer, "%d %f", OK, balance);
                        sendString(data->connection_fd, buffer, BUFFER_SIZE);
                    }
                    break;
                case WITHDRAW:
                    if (!checkValidAccount(income_msg.account_to)) {
                        sprintf(buffer, "%d %f", NO_ACCOUNT, balance);
                        sendString(data->connection_fd, buffer, BUFFER_SIZE);
                    } else{
                        printf("Withdrawing from account: %d, amount: %f\n", income_msg.account_from, income_msg.amount);
                        printf("Original balance: %f\n", data->bank_data->account_array[income_msg.account_from].balance);
                        //Generate new balance
                        balance = withdraw(data, income_msg.account_from, income_msg.amount);
                        //Not enough credits
                        if (balance == -1)
                        {
                            printf("Client has insufficient credit\n");
                            sprintf(buffer, "%d %f", INSUFFICIENT, balance);
                            sendString(data->connection_fd, buffer, BUFFER_SIZE);
                        }
                        else
                        {
                            printf("Account balance: %f\n", data->bank_data->account_array[income_msg.account_from].balance);
                            sprintf(buffer, "%d %f", OK, balance);
                            sendString(data->connection_fd, buffer, BUFFER_SIZE);
                        }
                    }
                    break;
                case TRANSFER:
                    check1 = checkValidAccount(income_msg.account_from);
                    check2 = checkValidAccount(income_msg.account_to);
                    //If both accounts exists
                    if (check1 == 1 && check2==1)
                    {
                        printf("From: %f\n", data->bank_data->account_array[income_msg.account_from].balance);
                        printf("Amount: %f\n", income_msg.amount);
                        //If amount is greater than balance
                        if (income_msg.amount > data->bank_data->account_array[income_msg.account_from].balance)
                        {
                            printf("Client has insufficient fonds\n");
                            sprintf(buffer, "%d %f", INSUFFICIENT, balance);
                            sendString(data->connection_fd, buffer, BUFFER_SIZE);
                        }
                        else{
                            printf("Making transfer from account %d to account %d\n", income_msg.account_from, income_msg.account_to);
                            //Transfer credits from accounts
                            transfer(data, income_msg.account_from, income_msg.account_to, income_msg.amount);
                            printf("New from balance: %f\n", data->bank_data->account_array[income_msg.account_from].balance);
                            printf("New to balance: %f\n", data->bank_data->account_array[income_msg.account_to].balance);
                            //Get new balance
                            balance = data->bank_data->account_array[income_msg.account_from].balance;
                            sprintf(buffer, "%d %f", OK, balance);
                            sendString(data->connection_fd, buffer, BUFFER_SIZE);
                        }
                    } else{
                        printf("Invalid account.\n");
                        sprintf(buffer, "%d %f", NO_ACCOUNT, balance);
                        sendString(data->connection_fd, buffer, BUFFER_SIZE);
                    }
                    break;
                case EXIT:
                    printf("Client is leaving\n");
                    break;
                default:
                    printf("Invalid option, please choose a correct option.\n");
                    continue;
            }

        }
    }

    //If server was interrupted send message
    if(interrupt_exit)
    {
        printf("Server shutting down...\n");
        sprintf(buffer, "%d 0", BYE);
        sendString(data->connection_fd, buffer, strlen(buffer)+1);
    }

    // Free data
    free(data);

    pthread_exit(NULL);
}

/*
    Free all the memory used for the bank data
*/
void closeBank(bank_t * bank_data, locks_t * data_locks)
{
    printf("DEBUG: Clearing the memory for the thread\n");
    free(bank_data->account_array);
    free(data_locks->account_mutex);
}


/*
    Return true if the account provided is within the valid range,
    return false otherwise
*/
int checkValidAccount(int account)
{
    return (account >= 0 && account < MAX_ACCOUNTS);
}

void getMessage(msg_t* msg, char* buffer)
{
    sscanf(buffer, "%d %d %d %f", &msg->operation, &msg->account_from, &msg->account_to, &msg->amount);
}
//Function to get current balance of an account
float check_Balance(thread_data_t* data, int account)
{
    float balance = -1;
    pthread_mutex_lock(data->data_locks->account_mutex);
    pthread_mutex_lock(&data->data_locks->transactions_mutex);

    balance = data->bank_data->account_array[account].balance;
    data->bank_data->total_transactions++;

    pthread_mutex_unlock(data->data_locks->account_mutex);
    pthread_mutex_unlock(&data->data_locks->transactions_mutex);

    return balance;
}
//Function to make a deposit to an account
float deposit(thread_data_t* data, int account, float amount)
{
    float balance = -1;
    pthread_mutex_lock(data->data_locks->account_mutex);
    pthread_mutex_lock(&data->data_locks->transactions_mutex);

    balance = data->bank_data->account_array[account].balance + amount;
    data->bank_data->account_array[account].balance += amount;
    data->bank_data->total_transactions++;

    pthread_mutex_unlock(data->data_locks->account_mutex);
    pthread_mutex_unlock(&data->data_locks->transactions_mutex);

    return balance;
}
//Function to make a withdraw of an account
float withdraw(thread_data_t* data, int account, float amount)
{
    float balance = -1;
    pthread_mutex_lock(data->data_locks->account_mutex);
    pthread_mutex_lock(&data->data_locks->transactions_mutex);

    if (amount < data->bank_data->account_array[account].balance) {
        balance = data->bank_data->account_array[account].balance - amount;
        data->bank_data->account_array[account].balance -= amount;
        data->bank_data->total_transactions++;
    }

    pthread_mutex_unlock(data->data_locks->account_mutex);
    pthread_mutex_unlock(&data->data_locks->transactions_mutex);

    return balance;
}
//Function to transfer between accounts
void transfer(thread_data_t* data, int account_from, int account_to, float amount)
{
    pthread_mutex_lock(data->data_locks->account_mutex);
    pthread_mutex_lock(&data->data_locks->transactions_mutex);

    if (amount < data->bank_data->account_array[account_from].balance) {
        data->bank_data->account_array[account_from].balance -= amount;
        data->bank_data->account_array[account_to].balance += amount;
        data->bank_data->total_transactions ++;
    }

    pthread_mutex_unlock(data->data_locks->account_mutex);
    pthread_mutex_unlock(&data->data_locks->transactions_mutex);
}

