//Rodrigo Garcia Mayo
//A01024595
//Assignment 5 - Network Blackjack Game

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Sockets libraries
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
// My sockets library
#include "sockets.h"

// Constant values
#define MAX_QUEUE 5
#define BUFFER_SIZE 200
#define DEFAULT_PORT 8989
#define CARDS 13
#define BLACKJACK 21

// Function declarations

//Socket functionality
void usage(char * program);
void waitForConnections(int server_fd);

void communicationLoop(int connection_fd); //Game protocol
void start_game(int connection_fd, char dealer_cards[], char client_cards[], int* dealer_cards_count, int* client_cards_count, int* count, int* game_bet); //Function that does the initial setup for a game
char get_card(); //Function to receive a random card from the deck
int check_royalty(char card); //Checks the values of non-numbered cards
void print_dealer_cards(char cards[], int cards_count); //Print dealer cards
void print_client_cards(char cards[], int cards_count); //Print client cards
void client_turn(int connection_fd, char client_cards[], int* client_cards_count, int* count); //After setup if no one has a blackjack the game continues
int check_count(char dealer_cards[], int dealer_cards_count); //Gives the card count
void dealer_turn(char dealer_cards[], int* dealer_cards_count, int* dealer_count); //After the client has played, now its the dealers turn

int main(int argc, char * argv[])
{
    if (argc != 2)
    {
        usage(argv[0]);
    }
    
    int server_fd;
    
    server_fd = prepareServer(argv[1], MAX_QUEUE);

    // Start waiting for incoming connections
    waitForConnections(server_fd);
    
    return 0;
}

void usage(char * program)
{
    printf("Usage: %s {port_number}\n", program);
    exit(EXIT_SUCCESS);
}

void waitForConnections(int server_fd)
{
    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof client_address;
    char client_presentation[INET_ADDRSTRLEN];
    int client_fd;
    pid_t new_pid;
    
    while(1)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_addrlen); //Accept incoming client
        if (client_fd == -1)
        {
            close(server_fd);
            close(client_fd);
            perror("ERROR: listen");
        }
        
        new_pid = fork(); //Fork to enable multiple clients
        
        if (new_pid == 0)   // Child process
        {
            // Close the main port
            close(server_fd);
            inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, INET_ADDRSTRLEN); 
            printf("Connection from: %s, port %i\n", client_presentation, client_address.sin_port);

            // Start the communication loop
            communicationLoop(client_fd);

            printf("Connection finalized from: %s, port %i\n", client_presentation, client_address.sin_port);
            
            // Terminate the child process
            close(client_fd);
            exit(EXIT_SUCCESS);
        }
        else if (new_pid > 0)   // Parent process
        {
            close(client_fd);
        }
        else
        {
            perror("ERROR: fork");
            exit(EXIT_FAILURE);
        }
    }
    
    // Close the server port
    close(server_fd);
}

void communicationLoop(int connection_fd)
{
    char buffer[BUFFER_SIZE];
    char dealer_cards[BLACKJACK];
    char client_cards[BLACKJACK];
    int dealer_cards_count = 0; //number of cards - dealer
    int client_cards_count = 0; //number of cards - client
    int client_money = 0;
    int game_bet = 0;
    int client_count = 0; //Count of cards - client
    int dealer_count = 0; //Count of cards - dealer
    char keep_playing = 'Y'; //Variable to permit multiple games

    printf("Game started.\n");

    recvMessage(connection_fd, buffer, BUFFER_SIZE); //Receive client money
    client_money = atoi(buffer);

    start_game(connection_fd, dealer_cards, client_cards, &dealer_cards_count, &client_cards_count, &dealer_count, &game_bet);
    recvMessage(connection_fd, buffer, BUFFER_SIZE); //rcv client initial count
    client_count = atoi(buffer);
    sprintf(buffer, "%d", dealer_count);
    send(connection_fd, buffer, BUFFER_SIZE, 0); //send dealer count
    if (client_count != 21) //Initial client blackjack
    {
        if (dealer_count != 21) //Initial dealer blackjack
        {
            client_turn(connection_fd, client_cards, &client_cards_count, &client_count);

            dealer_turn(dealer_cards, &dealer_cards_count, &dealer_count);
            sprintf(buffer, "%d", dealer_count);
            send(connection_fd, buffer, BUFFER_SIZE, 0); //Send dealer final count
            recvMessage(connection_fd, buffer, BUFFER_SIZE); //Receive player final count
            client_count = atoi(buffer);
            printf("Player final count was: %d\n", client_count);
            printf("Dealer final count was: %d\n", dealer_count);
        }
    }
    recvMessage(connection_fd, buffer, BUFFER_SIZE); //recv client money after game
    client_money = atoi(buffer);
    recvMessage(connection_fd, buffer, BUFFER_SIZE); //recv continue playing
    keep_playing = buffer[0];
    printf("Game ended.\n");
    //If client wants to play another game
    if (keep_playing == 'Y')
        communicationLoop(connection_fd);
    else
        printf("Client has left the game with %d credits.\n", client_money);
    close(connection_fd);
}

void dealer_turn(char dealer_cards[], int* dealer_cards_count, int* dealer_count)
{
    while (*dealer_count <= 17) //Dealer will settle when his cards count is greater or equal than 17
    {
        char card;
        //Dealer cards
        card  = get_card(); //Get a card
        dealer_cards[*dealer_cards_count] = card; //Add it to his other cards
        *dealer_cards_count = *dealer_cards_count + 1; //Sum the number of cards the dealer has

        print_dealer_cards(dealer_cards, *dealer_cards_count);
        *dealer_count = check_count(dealer_cards, *dealer_cards_count); //Get the count for the cards the dealer has
        if (*dealer_count > 21)
            break;
    }
}

void client_turn(int connection_fd, char client_cards[], int* client_cards_count, int* count)
{
    char buffer[BUFFER_SIZE];
    while (buffer[0] != 'N') //While the client wants cards
    {
        recvMessage(connection_fd, buffer, BUFFER_SIZE); //Want another card
        if (buffer[0] == 'Y')
        {
            char card;
            //Client cards
            card  = get_card(); //Get card
            client_cards[*client_cards_count] = card; //Add it to his deck
            *client_cards_count = *client_cards_count + 1; //Update his card #
            send(connection_fd, client_cards, BUFFER_SIZE, 0); //Send client cards
            print_client_cards(client_cards, *client_cards_count);
            *count = check_count(client_cards, *client_cards_count); //Check client count
            if (*count > 21)
                break;
        }
    }
}

void start_game(int connection_fd, char dealer_cards[], char client_cards[], int* dealer_cards_count, int* client_cards_count, int* count, int* game_bet)
{
    srand(time(NULL));
    char buffer[BUFFER_SIZE];
    char card;

    //Client cards
    card  = get_card();
    client_cards[*client_cards_count] = card;
    *client_cards_count = *client_cards_count + 1;
    card  = get_card();
    client_cards[*client_cards_count] = card;
    *client_cards_count = *client_cards_count + 1;
    //Dealer cards
    card = get_card();
    dealer_cards[*dealer_cards_count] = card;
    *dealer_cards_count = *dealer_cards_count + 1;
    card = get_card();
    dealer_cards[*dealer_cards_count] = card;
    *dealer_cards_count = *dealer_cards_count + 1;
    //Print the cards
    print_dealer_cards(dealer_cards, *dealer_cards_count);
    print_client_cards(client_cards, *client_cards_count);
    send(connection_fd, client_cards, BUFFER_SIZE, 0); //Send client cards
    send(connection_fd, dealer_cards, BUFFER_SIZE, 0); //Send dealer cards
    *count = check_count(dealer_cards, *dealer_cards_count);
    printf("Dealer count: %d\n", *count);
    recvMessage(connection_fd, buffer, BUFFER_SIZE); //Receive game bet
    *game_bet = atoi(buffer);
    printf("Game bet: %d\n", *game_bet);
}

char get_card()
{
    char deck[CARDS] = {'A', '2', '3', '4', '5', '6', '7', '8', '9', 'B', 'J', 'Q', 'K'}; //Deck of cards
    int card = rand() % (CARDS); //Get a random

    /*char deck[2] = {'A', 'K'};
    int card = rand() % (2);*/

    char card_char = deck[card]; //Get a random card from deck
    return  card_char;
}

void print_client_cards(char cards[], int cards_count) //Print client cards
{
    printf("Client cards:\n");
    printf("X\t"); //Server doesn't know the first client card
    for (int i=1;i<cards_count;i++)
    {
        if (cards[i] == '1')
            printf("A\t");
        else if (cards[i] == 'B')
            printf("10\t");
        else
            printf("%c\t", cards[i]);
    }
    printf("\n");
}

void print_dealer_cards(char cards[], int cards_count) //Print dealer cards
{
    printf("Dealer cards:\n");
    for (int i=0;i<cards_count;i++)
    {
        if (cards[i] == '1')
            printf("A\t");
        else if (cards[i] == 'B')
            printf("10\t");
        else
            printf("%c\t", cards[i]);
    }
    printf("\n");
}

int check_count(char dealer_cards[], int dealer_cards_count) //Gets the count of cards
{
    int a_count = -1;
    int count = 0;
    for (int i=0;i<dealer_cards_count;i++)
    {
        if (dealer_cards[i] == 'A')
            a_count = i;
        count += check_royalty(dealer_cards[i]);
    }
    if (count > 21 && a_count != -1) //If count > 21, make the first A count as 1 instead of 11
    {
        dealer_cards[a_count] = '1';
        count = check_count(dealer_cards, dealer_cards_count);
    }
    return count;
}

int check_royalty(char card) //Check non number cards value
{
    if (card == 'A')
        return 11;
    if (card == 'B')
        return 10;
    if (card == 'J')
        return 10;
    if (card == 'Q')
        return 10;
    if (card == 'K')
        return 10;
    if (card >= '1' && card <= '9')
        return (int) card - '0';

    return 0;
}
