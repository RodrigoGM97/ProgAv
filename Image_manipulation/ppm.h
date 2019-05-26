//Rodrigo Garcia Mayo
//A01024595
//Assignment 3 - Simple image manipulation

#ifndef PPM_H
#define PPM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct RGB
{
    unsigned char rgb[3];
}RGB;

typedef struct PPM
{
    int height;
    int width;
    char magic_number[3];
    int max_color;
    RGB** colors;
}PPM;

void mem_alloc(struct PPM* ppm); //Memory allocation for image

void free_mem(struct PPM* ppm); //Free memory from image

void image_read(char* file, struct PPM* ppm); //Read image from file

void image_write(char* file, struct PPM* ppm); //Write image

void ASCII_read(struct PPM * image, FILE * filename); //Read P3 image

void ASCII_write(const struct PPM * image, FILE * filename); //Write P3 image

void Binary_read(struct PPM * image, FILE * filename); //Reads P6 image

void Binary_write(const struct PPM * image, FILE * filename); //Write P6 image

void negative_image(PPM * image); //Converts image to negative

void scale_image(PPM* image, int scale); //Scale image to determined scale

void resize_image (struct PPM * final, struct PPM* image, int scale); //Resize image from scale

#endif



