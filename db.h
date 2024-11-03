#ifndef DB_H
#define DB_H

#include<errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>


#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100
#define PAGE_SIZE 4096


typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE+1];
    char email[COLUMN_EMAIL_SIZE+1];
} Row;

typedef struct{
    int file_descriptior;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGES];
}Pager;

typedef struct {
    uint32_t num_rows;
    Pager* pager;
} Table;

typedef struct{
    Table* table;
    uint32_t row_num;
    bool end_of_table;
}Cursor;

typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum { 
    META_COMMAND_SUCCESS,
    META_COMMAND_SHOW_ROWS, 
    META_COMMAND_UNRECOGNIZED_COMMAND 
} MetaCommandResult;

typedef enum { 
    STATEMENT_INSERT, 
    STATEMENT_SELECT 
} StatementType;

typedef struct {
    StatementType type;
    Row row_to_insert;  // only used by insert statement
} Statement;

typedef enum { 
    EXECUTE_SUCCESS, 
    EXECUTE_TABLE_FULL 
} ExecuteResult;

typedef enum { 
    PREPARE_SUCCESS, 
    PREPARE_SYNTAX_ERROR,
    PREPARE_STRING_TOO_LONG, 
    PREPARE_NEGATIVE_ID,
    PREPARE_UNRECOGNIZED_STATEMENT 
} PrepareResult;


// const 

extern const uint32_t ID_SIZE;
extern const uint32_t USERNAME_SIZE;
extern const uint32_t EMAIL_SIZE;
extern const uint32_t ID_OFFSET;
extern const uint32_t USERNAME_OFFSET;
extern const uint32_t EMAIL_OFFSET;
extern const uint32_t ROW_SIZE;
extern const uint32_t ROWS_PER_PAGE;
extern const uint32_t TABLE_MAX_ROWS;

// Function declarations
void print_row(Row* row);
void print_prompt();
void db_run(const char* filename);
void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);
void* cursor_value(Cursor* cursor);
InputBuffer* new_input_buffer();
void read_input(InputBuffer* inputbuffer);
void close_input_buffer(InputBuffer* input_buffer);
Table* db_open(const char* filename);
void* get_page(Pager* page,uint32_t page_num);
void db_close(Table* table);
void pager_flush(Pager* pager,uint32_t page_num,uint32_t size);
Pager* pager_open(const char* filename);
Cursor* table_start(Table* table);
Cursor* table_end(Table* table);
MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table* table);
PrepareResult prepare_insert(InputBuffer* input_buffer,Statement* statement);
PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement);
ExecuteResult execute_insert(Statement* statement, Table* table);
ExecuteResult execute_select(Statement* statement, Table* table);
ExecuteResult execute_statement(Statement* statement, Table* table);

#endif // DB_H