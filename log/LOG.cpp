#include "LOG.h"
   
#ifdef DEBUG_MODE 
    FILE* log_file = startLog(LOG_NAME);
#endif

FILE* startLog(const char* filename) {
    if (filename == NULL) return NULL;

    char dir_name[32] = {};
    sprintf(dir_name, "%s%s", LOG_DIR, filename);
    
    FILE* log_file = fopen(dir_name, "w+");

    if (log_file == NULL) {
        fprintf(stderr, "Can't open log file\n");
        return NULL;
    }

    fprintf(log_file, "<pre>\n");
    fprintf(log_file, "<p style=\"font-size:7px\">\n");

    //fprintf(log_file, "---------------------------Started logging---------------------------------\n");
    printTime(log_file);
    
    return log_file;
}

void printTime(FILE* file) {
    time_t t = time(NULL);

    struct tm tm = *localtime(&t);
    
    fprintf(file, "DATE: %d-%02d-%02d\n"
                  "TIME: %02d:%02d:%02d\n",
                 tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

}

int endLog() {
    if (log_file == NULL) return -1;

    fprintf(log_file, "\nEND OF LOG!\n");
    
    int fclose_report = fclose(log_file);
    if (fclose_report == EOF)
        return -1;

    return 0;
}
