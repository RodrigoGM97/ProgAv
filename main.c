//
// Created by rodrigo on 2/03/19.
//

#include <stdio.h>
#include <string.h>

void sum(int *a, int *b)
{
    *a = *a + *b;
}

void change(int arra[], int* c)
{
    arra[*c] = 4;
    *c = *c+1;
}

void same(int arra[], int arrb[], int len)
{
    memcpy(arrb, arra, len * sizeof(int));
    printf("Arrb: %d\n", arrb[2]);
}

void erase(int len)
{
    for (int i=0;i<len;i++)
        printf("\b");
}

int main()
{
    /*int a = 10;
    int b = 20;

    int arra[] = {1,2,3};
    int arrb[3];
    int c = 2;
    same(arra, arrb, sizeof(arra)/ sizeof(arra[0]));
    change(arra, &c);
    sum(&a, &b);
    printf("c: %d\n", c);
    printf("%d\n", arra[2]);
    printf("%d", a);*/

    printf("Quick brown fox");
    int i;
    //printf("\r");
    for(i=0; i < 9; i++)
    {
        printf("\b");
    }
    printf("green fox\n");

    //int arr[5] = {};

    return 0;
}