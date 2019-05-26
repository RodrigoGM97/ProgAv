//
// Created by rodrigo on 15/02/19.
//

#ifndef PROGAV_RAIL_FENCE_H
#define PROGAV_RAIL_FENCE_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

void create_Process(); //Initializes the parent and child process

void parent_process(int in_pipe[], int out_pipe[], int out_pipe_key[], int out_pipe_en_de[]); //Process that asks user for input and send input to child process

void child_process(int in_pipe[], int out_pipe[], int in_pipe_key[], int in_pipe_en_de[]); //Child process to encode/decode files

void prepare_pipes(int in_pipe[], int out_pipe[]); //Prepare the pipes to receive and send information

void close_pipes(int in_pipe[], int out_pipe[]); //Close the prepared pipes

void encode(char* msg, int key); //Function to encode a line

void decode(char* msg, int key); //Decodes rail fence line

#endif //PROGAV_RAIL_FENCE_H
