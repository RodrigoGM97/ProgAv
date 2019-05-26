//Rodrigo Garcia Mayo
//A01024595
//Assignment 3 - Simple image manipulation

#include "ppm.h"

void mem_alloc(struct PPM* image) //Memory allocation for image
{
    image->colors = malloc((image->height) * sizeof(PPM*));
    image->colors[0] = calloc((image->width) * (image->height), sizeof(PPM));
    for(int i = 1; i < (image->height); i++)
    {
        image->colors[i] = image->colors[0] + image->width * i;
    }
}

void free_mem(struct PPM* image) //Free memory from image
{
    free (image->colors[0]);
    free (image->colors);
}

void image_read(char* file_name, struct PPM* image) //Read image from file
{
    FILE *file = NULL;
    file = fopen(file_name, "r");
    //Check if you can open the file
    if(!file)
    {
        printf("Unable to open file %s\n", file_name);
        exit(-1);
    }
    fgets(image->magic_number, sizeof(image->magic_number), file); //Read magic number
    fscanf(file, "%d %d", &image->width, &image->height); //Read width and height
    fscanf(file, "%d", &image->max_color); //Read max color
    fgetc(file);

    mem_alloc(image); //Allocate memory for the image
    
    //Check if its a P3 or P6 file and read accordingly
    if (!strcmp("P3", image->magic_number))
    {
        ASCII_read(image, file);
    }
    else if (!strcmp("P6", image->magic_number))
    {
        Binary_read(image, file);
    }
    else
    {
        printf("The input header was not recognized\n");
    }
    
    fclose(file);
}

void ASCII_read(struct PPM * image, FILE * filename) //Read P3 image
{
    //Cycle image size
    for (int i=0; i<image->height; i++)
    {
        for (int j=0; j<image->width; j++)
        {
            fscanf (filename, "%hhu", &image->colors[i][j].rgb[0]); //Assign red
            fscanf (filename, "%hhu", &image->colors[i][j].rgb[1]); //Assign green
            fscanf (filename, "%hhu", &image->colors[i][j].rgb[2]); //Assign blue
        }
    }
}

void ASCII_write(const struct PPM * image, FILE * filename) //Write P3 image
{
    //Cycle image size
    for (int i=0; i<image->height; i++)
    {
        for (int j=0; j<image->width; j++)
        {
            fprintf (filename, "%3hhu ", image->colors[i][j].rgb[0]); //Write red
            fprintf (filename, "%3hhu ", image->colors[i][j].rgb[1]); //Write green
            fprintf (filename, "%3hhu\t", image->colors[i][j].rgb[2]); //Write blue
        }
        fprintf(filename, "\n");
    }
}

void Binary_read(struct PPM * image, FILE * filename) //Reads P6 image
{
    fread (image->colors[1], sizeof(RGB), image->width * image->height, filename); //Read all file character by character and store them in RGB struct
}

void Binary_write(const struct PPM * image, FILE * filename) //Write P6 image
{
    fwrite (image->colors[1], sizeof(RGB), image->width * image->height, filename); //Write the P6 file from struct
}

void image_write(char * filename, struct PPM * image) //Write image
{
    FILE * output_file = NULL;
    output_file = fopen (filename, "w");

    if (!output_file) //Check if file was created
    {
        printf("Unable to open the file '%s'\n", filename);
        exit(-1);
    }
    printf("Writing new image as: %s...\n", filename);
    //Write image header
    fprintf (output_file, "%s\n", image->magic_number);
    fprintf(output_file, "%d %d\n",image->width,image->height);
    fprintf (output_file, "%d\n", image->max_color);
    
    //Assign type of write
    if (!strcmp("P3", image->magic_number))
    {
        ASCII_write(image, output_file);
    }
    else if (!strcmp("P6", image->magic_number))
    {
        Binary_write(image, output_file);
    }

    fclose(output_file);
}

void negative_image(PPM * image) //Converts image to negative
{
    //Cycle through image size
    for (int i=0; i<image->height; i++)
    {
        for (int j=0; j<image->width; j++)
        {
            //Calculate new values for each color
            image->colors[i][j].rgb[0] = image->max_color - image->colors[i][j].rgb[0]; //Invert red
            image->colors[i][j].rgb[1] = image->max_color - image->colors[i][j].rgb[1]; //Invert green
            image->colors[i][j].rgb[2] = image->max_color - image->colors[i][j].rgb[2]; //Invert blue
        }
    }
}

void scale_image(struct PPM * image, int scale) //Scale image to determined scale
{
    PPM new_image; //New image
    
    //Transfer header to new image
    strncpy (new_image.magic_number, image->magic_number, 3);
    new_image.max_color = image->max_color;

    resize_image(&new_image, image, scale);

    free_mem(image); //Free old image
    *image = new_image; //Replace image
}

void resize_image (struct PPM * new_image, struct PPM* image, int scale) //Resize image from scale
{

    //Gets new image size
    new_image->height = image->height * (scale / 100.0);
    new_image->width = image->width * (scale / 100.0);

    mem_alloc(new_image); //Allocate memory for new_image
    
    //Calculate the resize factors
    double factor_x = image->width / (double) new_image->width;
    double factor_y = image->height / (double) new_image->height;

    //Cycle through new image size
    for (int r=0;r<new_image->height;r++)
    {
        for (int c=0;c<new_image->width;c++)
        {
            new_image->colors[r][c] = image->colors[(int)(r * factor_y)][(int)(c * factor_x)]; //Calculate pixel rgb values for new image
        }
    }
}

