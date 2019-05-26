#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#define MAX 100

void feedback (int x, int r);

int main() {
    srand(time(NULL));
    int r = rand() % MAX;
    int x;
    int i=0;

    while (i==0)
    {
        printf("Write a number: ");
        scanf("%d", &x);
        printf("You typed the number: %d\n", x);

        if (r == x)
        {
            printf("Number guessed!\n");
            i = 1;
        }
        else
        {
            printf("Incorrect number, try again!\n");
            feedback(x,r);
        }

    }
    return 0;
}

void feedback (int x, int r)
{
    if (x < r)
        printf("Number is bigger...\n");
    else
        printf("Number is smaller...\n");
}