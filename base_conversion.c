//Rodrigo Garcia Mayo
//A01024595
//Assignment 1 - Numerical base conversion

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#define ASCII_NUM_START 48
#define ASCII_LETTER_START 65

void base_ten_to_base_x (int decimal_number, int base_x); //converts a number from base ten to any other base
int base_x_to_base_ten (char* base_number, int base_x); //converts a number from any base to base ten

void base_ten_to_base_x(int decimal_number, int base_x) //converts a number from base ten to any other base
{
    int *remainder_array; //To store all the remainders every time you divide
    remainder_array = (int*)malloc(255);
    int remainder_array_it = 0; //The remainder_array iterator
    while(decimal_number>=base_x) //Until de divided number is lower than the base you are transforming it to
    {
        remainder_array[remainder_array_it] = decimal_number % base_x; //Add remainder to array
        decimal_number = decimal_number / base_x; //Divide the number
        remainder_array_it++;
    }
    remainder_array[remainder_array_it] = decimal_number; //Save the last remainder
    for(int i=remainder_array_it;i>=0;i--) //Iterate the saved remainders and print them backwards
    {
        if (remainder_array[i] > 9 && remainder_array[i]< 36) //For bases greater than 10 change the numbers for letters
        {
            remainder_array[i] = remainder_array[i] + 55;
            printf("%c", (char)remainder_array[i]); //Printing the results
        }
        else
            printf("%d",remainder_array[i]);//Printing the results
    }
    printf("\n");
}

int base_x_to_base_ten(char *base_number, int base_x) //converts a number from any base to base ten
{
    int actual_number; //Saves the analyzed number at the time
    int character_it = strlen(base_number)-1; //Iterate through the char array which has the number
    int base_ten_number=0; //Saves the number converted
    int exponent = 0; //Saves the current exponent
    while(character_it>=0) //Iterate through the whole number char by char
    {
        if(base_number[character_it] >= '0' && base_number[character_it] <= '9') //converts the ascii chars to numbers
            actual_number = base_number[character_it] - ASCII_NUM_START;
        if(base_number[character_it] >= 'A' && base_number[character_it] <= 'Z') //converts the ascii chars to numbers
            actual_number = base_number[character_it] - ASCII_LETTER_START + 10;
        base_ten_number += actual_number * pow(base_x,exponent); //Base changing method
        exponent++;
        character_it--;
    }
    return base_ten_number;
}

int main(int argc, char **argv)
{
    int option; //Input Output option for program arguments
    int base_from; //Base input
    int base_to; //Base output
    int base_ten = 0; //base ten number

    while ((option = getopt(argc, argv, "i:o:")) != -1) //Read I/O and base conversion
    {
        switch (option)
        {
            case 'i':
                base_from = atoi(optarg); //Read base input
                break;
            case 'o':
                base_to = atoi(optarg); //Read base output
                break;
            default:
                printf("Invalid input, try again.\n"); //If there's an incorrect input
                return -1;
        }
    }

    for (int i=optind;i<argc;i++) //Reads and converts the numbers from input
    {
        printf("The number %s base %d converted to base %d is: ", argv[i], base_from, base_to);
        base_ten = base_x_to_base_ten(argv[i], base_from); //Convert to base ten
        base_ten_to_base_x(base_ten, base_to); //Converts to Output base
    }
    return 0;
}