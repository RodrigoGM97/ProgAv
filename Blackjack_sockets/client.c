//Rodrigo Garcia Mayo
//A01024595
//Assignment 5 - Network Blackjack Game

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// Sockets libraries
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// My sockets library
#include "sockets.h"

// Constant values
#define BUFFER_SIZE 200
#define DEFAULT_PORT 8989
#define BLACKJACK 21

// Function declarations
void usage(char * program);
void blackjack(int connection_fd, int* initial_money); //Game protocol
void start_game(int connection_fd, char dealer_cards[], char client_cards[], int* dealer_cards_count, int* client_cards_count, int* count, int* game_bet, int initial_money); //Function that does the initial setup for a game
int check_royalty(char card); //Checks the values of non-numbered cards
void print_client_cards(char cards[], int cards_count); //Print client cards
void print_dealer_cards(char cards[], int cards_count, int hide); //Print dealer cards
void client_turn(int connection_fd, char client_cards[], int* client_cards_count, int* count); //After setup if no one has a blackjack the game continues
int check_count(char client_cards[], int client_cards_count); //Gives the card count

int main(int argc, char * argv[])
{
    int initial_money = 0;

    if (argc != 3)
    {
        usage(argv[0]);
    }
    
    int connection_fd;
    
    connection_fd = connectToServer(argv[1], argv[2]);

    printf("Place your initial currency...\n"); //Get your money
    scanf("%d", &initial_money);
    blackjack(connection_fd, &initial_money); //Join the table
    
    return 0;
}

void usage(char * program)
{
    printf("Usage: %s {server_address} {port_number}\n", program);
    exit(EXIT_SUCCESS);
}

void blackjack(int connection_fd, int* initial_money)
{
    //Check if you have money to play
    if (*initial_money > 0)
    {
        char buffer[BUFFER_SIZE];
        char dealer_cards[BLACKJACK];
        char client_cards[BLACKJACK];
        int dealer_cards_count = 0; //number of cards - dealer
        int client_cards_count = 0; //number of cards - client
        int client_count = 0; //Count of cards - client
        int dealer_count = 0; //Count of cards - dealer

        int game_bet = 0;
        char keep_playing = 'Y';

        printf("Game started.\n");
        sprintf(buffer, "%d", *initial_money);
        send(connection_fd, buffer, BUFFER_SIZE, 0);  //Send initial money
        start_game(connection_fd, dealer_cards, client_cards, &dealer_cards_count, &client_cards_count, &client_count, &game_bet, *initial_money);
        sprintf(buffer, "%d", client_count);
        send(connection_fd, buffer, BUFFER_SIZE, 0); //send client initial count
        recvMessage(connection_fd, buffer, BUFFER_SIZE); //recv dealer initial count
        dealer_count = atoi(buffer);

        //Check if there was a blackjack at the start of the game
        if (client_count == BLACKJACK || dealer_count == BLACKJACK)
        {
            if (dealer_count != BLACKJACK && client_count == BLACKJACK)
            {
                *initial_money += floor(game_bet * 1.5);
                printf("---------------------------------\n");
                printf("Dealer final count: %d\n", dealer_count);
                print_dealer_cards(dealer_cards, dealer_cards_count, 0);
                printf("Your final count: %d\n", client_count);
                print_client_cards(client_cards, client_cards_count);
                printf("You won!\n");
            }
            else if (dealer_count == BLACKJACK && client_count != BLACKJACK)
            {
                *initial_money -= game_bet;
                printf("---------------------------------\n");
                printf("Dealer final count: %d\n", dealer_count);
                print_dealer_cards(dealer_cards, dealer_cards_count, 0);
                printf("Your final count: %d\n", client_count);
                print_client_cards(client_cards, client_cards_count);
                printf("You lost!\n");
            }
            else if (dealer_count == BLACKJACK && client_count == BLACKJACK)
            {
                printf("---------------------------------\n");
                printf("Dealer final count: %d\n", dealer_count);
                print_dealer_cards(dealer_cards, dealer_cards_count, 0);
                printf("Your final count: %d\n", client_count);
                print_client_cards(client_cards, client_cards_count);
                printf("Tied, all bets are returned.\n");
            }

        }
        else {
            client_turn(connection_fd, client_cards, &client_cards_count, &client_count);
            sprintf(buffer, "%d", client_count);
            send(connection_fd, buffer, BUFFER_SIZE, 0); //Send client count
            recvMessage(connection_fd, buffer, BUFFER_SIZE);  //Receive dealer final count
            dealer_count = atoi(buffer);
            printf("---------------------------------\n");
            printf("Dealer final count: %d\n", dealer_count);
            print_dealer_cards(dealer_cards, dealer_cards_count, 0);
            printf("Your final count: %d\n", client_count);
            print_client_cards(client_cards, client_cards_count);
            //Check who won the game and modify client money
            if ((dealer_count < client_count && client_count <= BLACKJACK) || (dealer_count > BLACKJACK && client_count <= BLACKJACK))
            {
                printf("You won!\n");
                *initial_money += game_bet;
            }
            else if((dealer_count > client_count && dealer_count <= BLACKJACK) || (dealer_count > client_count && client_count > BLACKJACK) || (dealer_count <= BLACKJACK && client_count > BLACKJACK))
            {
                printf("You lost!\n");
                *initial_money -= game_bet;
            }
            else if (dealer_count == client_count || (client_count > BLACKJACK && dealer_count>BLACKJACK))
                printf("The game was a tie.\n");
        }

        printf("Game ended.\n");
        printf("Your current balance: %d\n", *initial_money);
        printf("Do you want to keep playing? (Y/N)\n");
        sprintf(buffer, "%d", *initial_money);
        send(connection_fd, buffer, BUFFER_SIZE, 0); //Send client money
        scanf(" %c", &keep_playing);
        buffer[0] = keep_playing;
        send(connection_fd, buffer, BUFFER_SIZE, 0); //Send keep playing

        //Another game?
        if (keep_playing == 'Y')
            blackjack(connection_fd, initial_money);
        else
            printf("You left the game with %d credits.\n", *initial_money);
    }
    else
        printf("Sorry, you don't have enough money to play.\n");

    // Close the socket to the client
    close(connection_fd);
}

void start_game(int connection_fd, char dealer_cards[], char client_cards[], int* dealer_cards_count, int* client_cards_count, int* count, int* game_bet, int initial_money)
{
    char buffer[BUFFER_SIZE];

    recvMessage(connection_fd, buffer, BUFFER_SIZE); //Receive client cards
    *client_cards_count = 2;
    memcpy(client_cards, buffer, *client_cards_count * sizeof(int));
    recvMessage(connection_fd, buffer, BUFFER_SIZE); //Receive dealer cards
    *dealer_cards_count = 2;
    memcpy(dealer_cards, buffer, *dealer_cards_count * sizeof(int));
    print_dealer_cards(dealer_cards, *dealer_cards_count, 1);
    print_client_cards(client_cards, *client_cards_count);
    *count = check_count(client_cards, *client_cards_count);
    printf("Your card count: %d\n", *count);
    printf("How much do you want to bet? (You have: %d)\n", initial_money);
    scanf("%d", game_bet);
    //Check if the client is can bet with the money he has
    while (*game_bet > initial_money)
    {
        printf("Your bet is greater than the money you have, place a correct bet.\n");
        scanf("%d", game_bet);
    }
    sprintf(buffer, "%d", *game_bet);
    send(connection_fd, buffer, BUFFER_SIZE, 0); //Send the game bet
}

void client_turn(int connection_fd, char client_cards[], int* client_cards_count, int* count)
{
    char buffer[BUFFER_SIZE];
    char more_cards = '0';
    //While player is playing
    while (more_cards != 'N')
    {
        printf("Do you want another card? (Y/N)\n");
        scanf(" %c", &more_cards);
        buffer[0] = more_cards;
        send(connection_fd, buffer, BUFFER_SIZE, 0); //Send if player wants another card
        if (more_cards == 'Y')
        {
            recvMessage(connection_fd, buffer, BUFFER_SIZE); //Receive client cards
            *client_cards_count = strlen(buffer);
            memcpy(client_cards, buffer, *client_cards_count * sizeof(int));
            print_client_cards(client_cards, *client_cards_count); //Print cards
            *count = check_count(client_cards, *client_cards_count); //Check new count
            printf("Your card count: %d\n", *count);
            //If player count surpasses 21 he can no longer ask for cards
            if (*count > 21)
                break;
        }
    }
}

void print_dealer_cards(char cards[], int cards_count, int hide) //Print dealer cards
{
    //printf("Dealer card count: %d\n", cards_count);
    printf("Dealer cards:\n");
    if (hide == 1)
        printf("X\t"); //The client cant see the first card of the dealer
    for (int i=hide;i<cards_count;i++)
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

void print_client_cards(char cards[], int cards_count) //Print client cards
{
    //printf("Client card count: %d\n", cards_count);
    printf("Client cards:\n");
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

int check_count(char client_cards[], int client_cards_count) //Gets the count of cards
{
    int a_count = -1;
    int count = 0;
    for (int i=0;i<client_cards_count;i++)
    {
        if (client_cards[i] == 'A')
            a_count = i;
        count += check_royalty(client_cards[i]);
    }
    if (count > 21 && a_count != -1) //If count > 21, make the first A count as 1 instead of 11
    {
        client_cards[a_count] = '1';
        count = check_count(client_cards, client_cards_count);
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
