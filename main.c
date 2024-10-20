#include<stdbool.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include <sys/types.h>

typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
}InputBuffer;

typedef enum{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
}ExecuteResult;

typedef enum{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND,
    META_COMMAND_SHOW_ROWS
} metaCommandResult;

typedef enum{
    PREPARE_SUCCESS,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
}PrepareResult;

typedef enum{
    STATEMENT_INSERT,
    STATEMENT_SELECT
}StatementType;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct{
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
}Row;


typedef struct{
    StatementType type;
    Row row_to_insert;
}Statement;

#define size_of_attributes(Struct,Attributes) sizeof(((Struct*)0)->Attributes);
const uint32_t ID_SIZE = size_of_attributes(Row,id);
const uint32_t USERNAME_SIZE = size_of_attributes(Row,username);
const uint32_t EMAIL_SIZE = size_of_attributes(Row,email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_SIZE + ID_OFFSET;
const uint32_t EMAIL_OFFSET = USERNAME_SIZE + USERNAME_OFFSET;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

typedef struct{
    uint32_t num_rows;
    void* pages[TABLE_MAX_PAGES];
}Table;

void print_row(Row* row){
    printf("(%d, %s, %s)\n",row->id,row->username,row->email);
}


void serialize_row(Row* source,void* destination){
    memcpy(destination+ID_OFFSET,&(source)->id,ID_SIZE);
    memcpy(destination+USERNAME_OFFSET,&(source)->username,USERNAME_SIZE);
    memcpy(destination+EMAIL_OFFSET,&(source)->email,EMAIL_SIZE);
}

void deserialize_row(void* source,Row* destination){
    memcpy(&(destination->id),source+ID_OFFSET,ID_SIZE);
    memcpy(&(destination)->username,source+USERNAME_OFFSET,USERNAME_SIZE);
    memcpy(&(destination)->email,source+EMAIL_OFFSET,EMAIL_SIZE);
}

void* row_slot(Table* table , uint32_t row_num){
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = table -> pages[page_num];
    if(page == NULL){
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page+byte_offset;
}

Table* new_table(){
    Table* table = (Table*)malloc(sizeof(Table));
    table->num_rows = 0;
    for(uint32_t i = 0;i < TABLE_MAX_PAGES;i++){
        table->pages[i] = NULL;
    }
    return table;
}

void free_table(Table* table){
    for(uint32_t i = 0;i < TABLE_MAX_PAGES;i++){
        free(table->pages[i]);
    }
    free(table);
}

InputBuffer* new_input_buffer(){
    InputBuffer* inputbuffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    inputbuffer -> buffer = NULL;
    inputbuffer -> buffer_length = 0;
    inputbuffer -> input_length = 0;
    return inputbuffer;
}

metaCommandResult do_meta_command(InputBuffer* inputbuffer,Table* table){
    if(strcmp(inputbuffer->buffer,".exit") == 0){
        exit(EXIT_SUCCESS);
    }else if(strcmp(inputbuffer->buffer,".rows") == 0){
        printf("Table has %u rows\n",table->num_rows);
        return META_COMMAND_SUCCESS;
    }else{
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}


PrepareResult prepare_statement(InputBuffer* inputbuffer,Statement* statement){
    if(strncmp(inputbuffer->buffer,"insert",6) == 0){
        statement -> type = STATEMENT_INSERT;
        int args_assigned = sscanf(inputbuffer->buffer,"insert %d %s %s",
        &statement->row_to_insert.id,&statement->row_to_insert.username,&statement->row_to_insert.email);
        if(args_assigned < 3){
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if(strcmp(inputbuffer->buffer,"select") == 0){
        statement -> type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_insert(Statement* Statement,Table* table){
    if(table->num_rows >= TABLE_MAX_ROWS){
        return EXECUTE_TABLE_FULL;
    }
    Row* row_to_insert = &(Statement->row_to_insert);
    serialize_row(row_to_insert,row_slot(table,table->num_rows));
    table->num_rows++;
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement,Table* table){
    Row row;
    for(uint32_t i = 0;i < table->num_rows;i++){
        deserialize_row(row_slot(table,i),&row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement,Table* table){
    switch (statement -> type){
        case(STATEMENT_INSERT):
            return execute_insert(statement,table);
        case(STATEMENT_SELECT):
            return execute_select(statement,table);
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
    Table* table = new_table();
    InputBuffer* inputbuffer = new_input_buffer();
    while(true){
        print_prompt();
        read_input(inputbuffer);
        if(inputbuffer -> buffer[0] == '.'){
            switch (do_meta_command(inputbuffer,table)){
                case (META_COMMAND_SUCCESS):
                    continue;
                case (META_COMMAND_SHOW_ROWS):
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
            case (PREPARE_SYNTAX_ERROR):
                printf("Syntex error.Could not parse statement.\n");
                continue;
            case (PREPARE_UNRECOGNIZED_STATEMENT):
                printf("Unrecognized keyword at start of '%s'.\n",inputbuffer->buffer);
                continue;
        }
        switch (execute_statement(&statement,table))
        {
        case (EXECUTE_SUCCESS):
            printf("Executed.\n");
            break;
        
        case(EXECUTE_TABLE_FULL):
            printf("Error: Table full.\n");
            break;
        }

    
    }
}