#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define DEBUG_MODE 1

#define LOG_NAME "logs.txt"

extern FILE* log_file;

#ifndef PRINT_FILE
    #define PRINT_FILE "obj/log.htm"
#endif

#define LOG_DIR "obj/"

#define FOR_LOGS FILE* log_file

#define LOG_PARAMS \
        const char* FUNC_NAME, const char* FILE_NAME, int _LINE_, FILE* log_file 

#define LOG_ARGS \
        __FUNCTION__, __FILE__, __LINE__, log_file

#define LOGS_ARGS_USE \
        FUNC_NAME, FILE_NAME, _LINE_, log_file

const size_t FILE_NAME_SIZE = 32;

FILE* startLog (const char* filename);
int   endLog   ();
void  printTime(FILE* file);

