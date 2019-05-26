//Rodrigo Garcia Mayo
//A01024595
//Assignment 4 - Simple image manipulation

#include "rail_fence.h"
#define BUFFER_SIZE 10000

void create_Process() //Initializes the parent and child process
{
    pid_t new_pid;
    int parent_to_child_msg[2]; //pipe to send msg to child
    int parent_to_child_key[2]; //pipe to send key to child
    int parent_to_child_en_de[2]; //pipe to send if user wants to encode or decode
    int child_to_parent[2]; //pipe to send msg from child to parent
    //Pipe creation
    if (pipe(parent_to_child_msg) == -1)
    {
        perror("Parent to child pipe error.");
        exit(EXIT_FAILURE);
    }
    if (pipe(parent_to_child_key) == -1)
    {
        perror("Parent to child pipe error.");
        exit(EXIT_FAILURE);
    }
    if (pipe(parent_to_child_en_de) == -1)
    {
        perror("Parent to child pipe error.");
        exit(EXIT_FAILURE);
    }
    if (pipe(child_to_parent) == -1)
    {
        perror("Child to parent pipe error.");
        exit(EXIT_FAILURE);
    }

    new_pid = fork(); //Process creation

    if (new_pid > 0) //Parent process
    {
        int status;
        parent_process(child_to_parent, parent_to_child_msg, parent_to_child_key, parent_to_child_en_de); //Parent process initialization
        wait(&status); //Parent waits for child
    }
    else if (new_pid == 0) //Child process
    {
        child_process(parent_to_child_msg, child_to_parent, parent_to_child_key, parent_to_child_en_de); //Child process initialization
    }
    else //Process creation error
    {
        printf("ERROR: unable to fork\n");
        exit(EXIT_FAILURE);
    }
}

void prepare_pipes(int in_pipe[], int out_pipe[]) //Prepare the pipes to receive and send information
{
    close(in_pipe[1]);
    close(out_pipe[0]);
}

void close_pipes(int in_pipe[], int out_pipe[]) //Close the prepared pipes
{
    close(in_pipe[0]);
    close(out_pipe[1]);
}

void parent_process(int in_pipe[], int out_pipe[], int out_pipe_key[], int out_pipe_en_de[]) //Process that asks user for input and send input to child process
{
    char filename[BUFFER_SIZE]; //Input filename
    char output_filename[BUFFER_SIZE];
    char key[BUFFER_SIZE]; //Key for rail fence
    char encode_decode[BUFFER_SIZE]; //Option to encode or decode
    int exit = 1; //Variable to finish program
    while (exit != 0) //While the user still wants to enter input
    {
        printf("Do you want to encode or decode? [e/d]\n");
        scanf("%s", encode_decode);
        printf("Input file name: \n");
        scanf("%s", filename);
        printf("Input key: \n");
        scanf("%s", key);
        prepare_pipes(in_pipe, out_pipe); //Prepare file pipe
        prepare_pipes(in_pipe, out_pipe_key); //Prepare key pipe
        prepare_pipes(in_pipe, out_pipe_en_de); //Prepare encode/decode option pipe
        write(out_pipe[1], filename, strlen(filename)); //Send input file to child
        write(out_pipe_key[1], key, strlen(key)); //Send key to child
        write(out_pipe_en_de[1], encode_decode, strlen(encode_decode)); //Send E/D option to child

        read(in_pipe[0], output_filename, BUFFER_SIZE); //Get the name of the encoded/decoded file
        printf("File encoded with name: %s\n", output_filename);
        printf("¿Do you want to continue? [1-Yes, 0- No]\n");
        scanf("%d", &exit);
        //Close pipes after use
        close_pipes(in_pipe, out_pipe);
        close_pipes(in_pipe, out_pipe_key);
        close_pipes(in_pipe, out_pipe_en_de);
    }
}

void child_process(int in_pipe[], int out_pipe[], int in_pipe_key[], int in_pipe_en_de[]) //Child process to encode/decode files
{
    char input_file_name[BUFFER_SIZE];
    char output_file_name[BUFFER_SIZE];
    char msg[BUFFER_SIZE];
    char key[BUFFER_SIZE];
    char encode_decode[BUFFER_SIZE];
    int len = 0; //Length of msg
    //Prepare pipes to receive information
    prepare_pipes(in_pipe, out_pipe);
    prepare_pipes(in_pipe_key, out_pipe);
    prepare_pipes(in_pipe_en_de, out_pipe);
    //Read the filename, key and E/D option
    read(in_pipe[0], input_file_name, BUFFER_SIZE);
    read(in_pipe_key[0], key, BUFFER_SIZE);
    read(in_pipe_en_de[0], encode_decode, BUFFER_SIZE);

    FILE *input_file = fopen(input_file_name, "r"); //Pointer to input file
    if(!input_file)
    {
        printf("Unable to open file %s\n", input_file_name);
        exit(-1);
    }
    FILE * output_file = NULL; //Pointer to output file
    //Set name of file
    if (strcmp(encode_decode, "e") == 0)
        strcpy(output_file_name, "encoded_");
    if (strcmp(encode_decode, "d") == 0)
        strcpy(output_file_name, "decoded_");
    strcat(output_file_name, input_file_name);
    output_file = fopen (output_file_name, "w");
    if (!output_file) //Check if file was created
    {
        printf("Unable to open the file '%s'\n", output_file_name);
        exit(-1);
    }

    while(!feof(input_file)) //Read file line by line
    {
        fgets(msg, BUFFER_SIZE, input_file); //Read a line from the file
        len = strlen(msg); //Length of line
        if (strcmp(encode_decode, "e") == 0) //Option to encode
        {
            encode(msg, atoi(key)); //Calls the encoding function
            fwrite(msg, len - 1, 1, output_file); //Write msg to new file
            if ((msg[len-1] >= 'a' && msg[len-1] <= 'z') || (msg[len-1] == '.')) //Adds last character to file
                fprintf(output_file, "%c", msg[len-1]);
            else
                fprintf(output_file, "\n");

        }
        if (strcmp(encode_decode, "d") == 0) //Option to decode
        {
            decode(msg, atoi(key)); //Decode line function
            fwrite(msg, len-1, 1, output_file); //Write the decoded line
            if ((msg[len-1] >= 'a' && msg[len-1] <= 'z') || (msg[len-1] == '.')) //Adds last character to file
                fprintf(output_file, "%c", msg[len-1]);
            else
                fprintf(output_file, "\n");
        }

    }

    write(out_pipe[1], output_file_name, BUFFER_SIZE); //Send output file to parent process
    //Closes pipes after use
    close_pipes(in_pipe, out_pipe);
    close_pipes(in_pipe_key, out_pipe);
    close_pipes(in_pipe_en_de, out_pipe);
}

void encode(char* msg, int key) //Function to encode a line
{
    strtok(msg, "\n"); //Ignore eol
    int msgLen = strlen(msg);
    int k = -1;
    int row = 0;
    int col = 0;
    int cont=0;
    char encrypted_str[msgLen];
    char railMatrix[key][msgLen]; //Matrix that stores the rail fence

    //Filling matrix with space
    for(int i = 0; i < key; i++) {
        for (int j = 0; j < msgLen; j++) {
            railMatrix[i][j] = '\n';
        }
    }
    //Fill the matrix with msg
    for(int i = 0; i < msgLen; i++){
        railMatrix[row][col++] = msg[i];
        if(row == 0 || row == key-1) //If the matrix gets to the rail boundaries (0 and key)
            k = k * (-1); //Change direction
        row = row + k; //Change row to next one
    }
    //Put the encoded msg in a line
    for(int i = 0; i < key; i++) {
        for (int j = 0; j < msgLen; j++) {
            if (railMatrix[i][j] != '\n') {
                encrypted_str[cont] = railMatrix[i][j];
                cont++;
            }
        }
    }
    strcpy(msg, encrypted_str); //Copy the encrypted string to the original msg
}

void decode(char* msg, int key) //Decodes rail fence line
{
    strtok(msg, "\n");
    int msgLen = strlen(msg);
    int k = -1; //Direction
    int row = 0;
    int col = 0;
    int m = 0;
    char decrypted_str[msgLen];
    char railMatrix[key][msgLen];

    //Fill matrix with space
    for(int i = 0; i < key; i++) {
        for (int j = 0; j < msgLen; j++) {
            railMatrix[i][j] = '\n';

        }
    }
    //Mark the places
    for(int i = 0; i < msgLen; i++)
    {
        railMatrix[row][col++] = '*'; //Mark

        if(row == 0 || row == key-1)
            k = k * (-1); //Change direction

        row = row + k; //Change row
    }

    //construct matrix
    for(int i = 0; i < key; i++) {
        for (int j = 0; j < msgLen; j++) {
            if (railMatrix[i][j] == '*')
                railMatrix[i][j] = msg[m++];
        }
    }

    row = col = 0;
    k = -1;

    int cont = 0;
    //Retrieve msg
    for(int i = 0; i < msgLen; ++i)
    {
        decrypted_str[cont] = railMatrix[row][col++]; //Get info
        cont++;
        if(row == 0 || row == key-1)
            k = k * (-1); //Change direction

        row = row + k; //Change actual row
    }
    strcpy(msg, decrypted_str); //Send message
}
