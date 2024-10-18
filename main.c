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

typedef enum{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} metaCommandResult;

typedef enum{
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
}PrepareResult;

typedef enum{
    STATEMENT_INSERT,
    STATEMENT_SELECT
}StatementType;

typedef struct{
    StatementType type;
}Statement;


InputBuffer* new_input_buffer(){
    InputBuffer* inputbuffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    inputbuffer -> buffer = NULL;
    inputbuffer -> buffer_length = 0;
    inputbuffer -> input_length = 0;
    return inputbuffer;
}

metaCommandResult do_meta_command(InputBuffer* inputbuffer){
    if(strcmp(inputbuffer->buffer,".exit") == 0){
        exit(EXIT_SUCCESS);
    }else{
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}


PrepareResult prepare_statement(InputBuffer* inputbuffer,Statement* statement){
    if(strncmp(inputbuffer->buffer,"insert",6) == 0){
        statement -> type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    }
    if(strcmp(inputbuffer->buffer,"select") == 0){
        statement -> type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

void execute_statement(Statement* statement){
    switch (statement -> type){
        case(STATEMENT_INSERT):
            printf("This is where we would do an insert.\n");
            break;
        case(STATEMENT_SELECT):
            printf("This is where we would do an select.\n");
            break;
    }
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
        if(inputbuffer -> buffer[0] == '.'){
            switch (do_meta_command(inputbuffer)){
                case (META_COMMAND_SUCCESS):
                    continue;
                case (META_COMMAND_UNRECOGNIZED_COMMAND):
                    printf("Unrecognized command '%s'\n",inputbuffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(inputbuffer,&statement)){
            case (PREPARE_SUCCESS):
                break;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of '%s'.\n",inputbuffer->buffer);
                continue;
        }
        execute_statement(&statement);
        printf("Executed.\n");
    }
}