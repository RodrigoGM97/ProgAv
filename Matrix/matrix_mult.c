//Rodrigo Garcia Mayo
//A01024595
//Assignment 2 - Matrix multiplication

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

struct Matrix file_read(char* file_name); //Read matrices from files
struct Matrix matrix_multiplication(struct Matrix a, struct Matrix b); //Multiply matrices
void file_write(struct Matrix result, char* file_name); //Write result matrix to file
int check_multiplication (int rows, int cols); //Verifies the matrices can be multiplied

struct Matrix
{
    int rows;
    int cols;
    float** matrix;
};

void free_mem(float **matrix, int n_rows) //Free memory from matrix
{
    for (int i=0;i<n_rows;i++) //Free received matrix
    {
        free(matrix[i]);
    }
    free(matrix);
}

struct Matrix file_read(char* file_name) //Read matrices from files
{
    int n_rows_a = 0; //Number of rows for the first matrix
    int n_cols_a = 0; //Number of columns for the first matrix

    float** matrix_a_arr = NULL; //First matrix

    float input_number = 0;

    FILE *file = fopen(file_name, "r"); //Open first file
    if (!file) {
        printf("Error opening %s\n", file_name);
    }

    fscanf (file, "%d %d", &n_rows_a, &n_cols_a); //Read number of rows and columns for first file

    matrix_a_arr = (float**) malloc((n_rows_a) *  sizeof(float*)); //Allocating memory for first matrix

    for (int i=0;i<n_rows_a;i++)
    {
        matrix_a_arr[i] = (float*) malloc(n_cols_a * sizeof(float*));
    }

    for (int rows=0;rows<n_rows_a;rows++) //Storing first matrix
    {
        for (int cols=0;cols<n_cols_a;cols++)
        {
            fscanf (file, "%f", &input_number);
            matrix_a_arr[rows][cols] =  input_number;
        }
    }

    //free_mem(matrix_a_arr, n_rows_a); //Free the matrix a

    fclose (file); //Close first file
    struct Matrix matrix;
    matrix.cols = n_cols_a;
    matrix.rows = n_rows_a;
    matrix.matrix = matrix_a_arr;
    return matrix;
}

int check_multiplication (int rows, int cols) //Verifies the matrices can be multiplied
{
    if (cols != rows)
        return -1; //error code
    else
        return 0;
}

struct Matrix matrix_multiplication(struct Matrix a, struct Matrix b) //Multiply matrices
{
    if (check_multiplication(b.rows,a.cols) == -1)
    {
        printf("Matrices can't be multiplied\n");
        exit(-1);
    }
    float** result_arr = NULL; //Multiplied matrix
    result_arr = (float**)malloc((1+a.rows)* sizeof(float*)); //Multiplied matrix storage allocation
    for (int i=0;i<a.rows;i++)
    {
        result_arr[i] = (float*) malloc((1+b.cols)* sizeof(float*));
    }
    for (int rows=0;rows<a.rows;rows++) //Multiplying the matrices and storing them in result_arr
    {
        for (int cols=0;cols<b.cols;cols++)
        {
            result_arr[rows][cols] = 0;
            for (int k=0;k<b.cols;k++)
            {
                result_arr[rows][cols] = result_arr[rows][cols] + a.matrix[rows][k] * b.matrix[k][cols];
            }
        }
    }

    struct Matrix result;
    result.matrix = result_arr;
    result.cols = b.cols;
    result.rows = a.rows;
    return result;
}

void file_write(struct Matrix result, char* file_name) //Write result matrix to file
{
    FILE *output; //Pointer to the file
    output = fopen(file_name, "w"); //Creating the result file
    printf("%d %d\n", result.rows, result.cols); //Printing number of rows and columns
    fprintf(output,"%d %d\n", result.rows, result.cols); //Writing number of rows and columns in file
    for (int rows=0;rows<result.rows;rows++) //Writing and printing contents of matrix
    {
        for (int cols=0;cols<result.cols;cols++)
        {
            printf("%0.2f\t",result.matrix[rows][cols]);
            fprintf (output, "%0.2f\t",result.matrix[rows][cols]);
        }
        printf("\n");
        fprintf (output, "\n");
    }
    fclose(output); //Close written file
}

int main(int argc, char **argv)
{
    int option = 0;
    char *result_file = NULL;
    struct Matrix matrix;
    struct Matrix matrix2;
    struct Matrix result;
    while ((option = getopt(argc, argv, "1:2:r:")) != -1) //Read I/O and base conversion
    {
        switch (option)
        {
            case '1':
                matrix = file_read(optarg); //Read first matrix
                break;
            case '2':
                matrix2 = file_read(optarg); //Read second matrix
                break;
            case 'r':
                result_file = optarg; //Read file output
                break;
            default:
                printf("Invalid input, try again.\n"); //If there's an incorrect input
                return -1;
        }
    }

    result = matrix_multiplication(matrix, matrix2); //Multiply matrices and store them
    file_write(result, result_file); //Write results in file
    free_mem(matrix.matrix, matrix.rows); //Free first matrix
    free_mem(matrix2.matrix, matrix2.rows); //Free second matrix
    free_mem(result.matrix, result.rows); //Free result matrix
    return 0;
}
