//Rodrigo Garcia Mayo
//A01024595
//Assignment 3 - Simple image manipulation

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "ppm.h"


void menu(char * in_filename, char * out_filename, int negative, int scale); //Start program

int main(int argc, char **argv)
{
    int option; //input_option
    char * input_file;
    char * output_file;
    int negative = 0; //don't do negative = 0, do negative = 1
    int scale = 0;
    
    while ( (option = getopt(argc, argv, "i:o:s:n")) != -1 )
    {
        switch (option)
        {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'n':
                negative = 1;
                break;
            case 's':
                scale = atoi(optarg);
                break;
            default:
                printf("Invalid input '%c'\n", option);
                break;
        }
    }
    menu(input_file, output_file, negative, scale); //Start program according to input

    return 0;
}

void menu(char* input_file, char* output_file, int negative, int scale) //Start program
{
    struct PPM image;
    image_read(input_file, &image); //Read image from file
    
    if (negative==1) // Get the negative of the image
    {
        negative_image(&image);
    }

    if(scale != 0 || scale!=100) //Scale image
    {
        scale_image(&image, scale);
    }

    image_write(output_file, &image); //Write image

    free_mem(&image); //Free memory
}

