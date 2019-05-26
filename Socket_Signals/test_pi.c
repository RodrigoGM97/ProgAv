//Rodrigo Garcia Mayo
//A01024595
//Assignment 7 - Sockets and signals

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "get_pi.h"

int main(int argc, char * argv[])
{
	unsigned long int iterations;
    double result;

	if (argc == 2)
	{
		// Get the number of iterations as the first parameter
        // strtoul receives a string, a pointer to a string
        //  to store the resulting string, and the base of the number
		iterations = strtoul(argv[1], NULL, 10);
	}
	else
    {
        printf("Usage:\n\t%s {num_iterations}\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    result = computePI(iterations);

    printf("The number PI with %lu iterations is %.20lf\n", iterations, result);
    printf("The difference with M_PI is %.20lf\n", M_PI - result);

	return 0;
}
