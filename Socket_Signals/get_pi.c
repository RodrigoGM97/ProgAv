//Rodrigo Garcia Mayo
//A01024595
//Assignment 7 - Sockets and signals

#include "get_pi.h"

double computePI(unsigned long int iterations)
{
    double result = 4;
    int sign = -1;
    unsigned long int divisor = 3;
    unsigned long int counter = 0;

    for (counter = 0; counter<iterations; counter++)
    {
        result += sign * (4.0/divisor);
        sign *= -1;
        divisor += 2;
    }

    return result;
}
