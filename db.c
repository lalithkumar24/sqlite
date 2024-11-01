#include "db.h"

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row,id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row,username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row,email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t ROWS_PER_PAGE = 14;
const uint32_t TABLE_MAX_ROWS = 1400;

void print_prompt(){
    printf("db -> ");
}
void print_row(Row* row) {
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

void serialize_row(Row* source, void* destination) {
    memcpy((char*)destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy((char*)destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy((char*)destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

void deserialize_row(void* source, Row* destination) {
    memcpy(&(destination->id), (char*) source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), (char*) source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email),(char*) source + EMAIL_OFFSET, EMAIL_SIZE);
}
void* get_page(Pager* pager,uint32_t page_num){
    if(page_num > TABLE_MAX_PAGES){
        printf("Page Out of bound. %d > %d\n",page_num,TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }
    if(pager->pages[page_num] == NULL){
        void* page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager -> file_length / PAGE_SIZE;
        if(pager -> file_length % PAGE_SIZE){
            num_pages += 1;
        }

        if(page_num <= num_pages){
            lseek(pager->file_descriptior,page_num*PAGE_SIZE,SEEK_SET);
            ssize_t bytes_read = read(pager->file_descriptior,page,PAGE_SIZE);
            if(bytes_read == -1){
                printf("Error reading file: %d\n",errno);
                exit(EXIT_FAILURE);
            }   
        }
        pager->pages[page_num] = page;
    }
    return pager->pages[page_num];

}
void* row_slot(Table* table, uint32_t row_num) {
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void* page = get_page(table->pager,page_num);
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return (char*)page + byte_offset;
}

InputBuffer* new_input_buffer() {
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
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
void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

void pager_flush(Pager* pager,uint32_t page_num,uint32_t size){
    if(pager->pages[page_num] == NULL){
        printf("Tried to flush null page\n");
        exit(EXIT_FAILURE);
    }
    off_t offset = lseek(pager->file_descriptior,page_num* PAGE_SIZE,SEEK_SET);
    if(offset == -1){
        printf("Error seeking: %d\n",errno);
        exit(EXIT_FAILURE);
    }

    ssize_t bits_written = write(pager->file_descriptior,pager ->pages[page_num],size);

    if(bits_written == -1){
        printf("Error written: %d\n",errno);
        exit(EXIT_FAILURE);
    }

}


Table* db_open(const char* filename) {
    Pager* pager = (Pager*) pager_open(filename);
    uint32_t num_rows = pager->file_length/ROW_SIZE;
    Table* table =(Table*) malloc(sizeof(Table));
    table -> pager = pager;
    table -> num_rows = num_rows;
    return table;
}


void db_close(Table* table){
    Pager* pager = table->pager;
    uint32_t num_full_pages = table->num_rows/ROWS_PER_PAGE;
    for(uint32_t i = 0;i < num_full_pages;i++){
        if(pager->pages[i] == NULL){
            continue;
        }
        pager_flush(pager,i,PAGE_SIZE);
        free(pager->pages[i]);
        pager -> pages[i] = NULL;
    }

    uint32_t num_additional_rows = table -> num_rows % ROWS_PER_PAGE;
    if(num_additional_rows > 0){
        uint32_t page_num = num_full_pages;
        if(pager->pages[page_num] != NULL){
            pager_flush(pager,page_num,num_additional_rows* ROW_SIZE);
            free(pager->pages[page_num]);
            pager->pages[page_num] = NULL;
        }
    }
    
    int result = close(pager->file_descriptior);
    if(result == -1){
        printf("Error closing DB file.\n");
        exit(EXIT_FAILURE);
    }
    for(uint32_t i = 0;i < TABLE_MAX_PAGES;i++){
        void* page = pager->pages[i];
        if(page){
           free(page); 
           pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);

}

Pager* pager_open(const char* filename){
    int fd = open(filename,O_RDWR | O_CREAT ,S_IWUSR | S_IRUSR);
    //printf("fd output:%d\n",fd);
    if(fd == -1){
        printf("Unable to open file\n");
        exit(EXIT_FAILURE);
    }   

    off_t file_length  = lseek(fd,0,SEEK_END);
    Pager* page = (Pager*)malloc(sizeof(Pager));
    page -> file_descriptior = fd;
    page -> file_length = file_length;
    for(int i = 0;i < TABLE_MAX_PAGES;i++){
        page->pages[i] = NULL;
    } 
    return page;
}


MetaCommandResult do_meta_command(InputBuffer* input_buffer, Table* table) {
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    }else if(strcmp(input_buffer->buffer, ".rows") == 0){
        printf("Table has %u rows\n",table->num_rows);
        return META_COMMAND_SHOW_ROWS;
    } 
    else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}
PrepareResult prepare_insert(InputBuffer* input_buffer,Statement* statement){
    statement->type = STATEMENT_INSERT;
    char* keyword = strtok(input_buffer->buffer," ");
    char* id_string = strtok(NULL," ");
    char* username = strtok(NULL," ");
    char* email = strtok(NULL," ");

    if(id_string == NULL || username == NULL || email == NULL){
        return PREPARE_SYNTAX_ERROR;
    }
    int id = atoi(id_string);
    if(id < 0){
        return PREPARE_NEGATIVE_ID;
    }
    if(strlen(username) > COLUMN_USERNAME_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }
    if(strlen(email) > COLUMN_EMAIL_SIZE){
        return PREPARE_STRING_TOO_LONG;
    }
    statement -> row_to_insert.id = id;
    strcpy(statement->row_to_insert.email,email);
    strcpy(statement->row_to_insert.username,username);
    return PREPARE_SUCCESS;
}

PrepareResult prepare_statement(InputBuffer* input_buffer, Statement* statement) {
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        return prepare_insert(input_buffer,statement);
    }
    if (strcmp(input_buffer->buffer, "select") == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_insert(Statement* statement, Table* table) {
    if (table->num_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }

    Row* row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows += 1;

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table* table) {
    Row row;
    for (uint32_t i = 0; i < table->num_rows; i++) {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement* statement, Table* table) {
    switch (statement->type) {
        case (STATEMENT_INSERT):
            return execute_insert(statement, table);
        case (STATEMENT_SELECT):
            return execute_select(statement, table);
    }
    return EXECUTE_SUCCESS;
}

void db_run(const char* filename){
    
    Table* table = db_open(filename);

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
            case(PREPARE_NEGATIVE_ID):
                printf("ID must be positive\n");
                continue;
            case(PREPARE_STRING_TOO_LONG):
                printf("String is too long\n");
                continue;
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