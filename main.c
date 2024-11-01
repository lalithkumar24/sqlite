#include "db.h"
int main(int argc, char* argv[]) {
    if(argc < 2){
        printf("Need a file run database\n");
        exit(EXIT_FAILURE);
    }
    const char* filename = argv[1];
    db_run(filename);
    return 0;
}