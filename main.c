#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>

typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
}InputBuffer;

InputBuffer* new_input_buffer(){
    InputBuffer* inputbuffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    inputbuffer -> buffer = NULL;
    inputbuffer -> buffer_length = 0;
    inputbuffer -> input_length = 0;
    return inputbuffer;
}

void print_prompt(){
    printf("db -> ");
}

void read_input(InputBuffer* inputbuffer){
    ssize_t bytes_read = getline(&(inputbuffer->buffer),&(inputbuffer->buffer_length),stdin);

    if(bytes_read <= 0){
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }

    inputbuffer -> input_length = bytes_read;
    inputbuffer -> buffer[bytes_read-1] = 0;
}

void close_input_buffer(InputBuffer* inputbuffer){
    free(inputbuffer -> buffer);
    free(inputbuffer);
}

int main(int argc,char* argv[]){
    InputBuffer* inputbuffer = new_input_buffer();
    while(true){
        print_prompt();
        read_input(inputbuffer);
        if(strcmp(inputbuffer->buffer,".exit") == 0){
            close_input_buffer(inputbuffer);
            exit(EXIT_SUCCESS);
        }else{
            printf("Unrecognized command '%s'.\n",inputbuffer->buffer);
        }
    }
}