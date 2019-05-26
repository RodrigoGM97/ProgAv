//Rodrigo Garcia Mayo
//A01024595
//Assignment 7 - Sockets and signals

#include "fatal_error.h"

void fatalError(const char * message)
{
    perror(message);
    exit(EXIT_FAILURE);
}
