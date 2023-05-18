#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define PATH_1   "run/x86_back"
#define PATH_2   "run/y86"
#define INPUT_PATH "tests/input.txt"
#define TEMP_PATH  "tests/temp_input.txt"

#define TEMP_TEST                        \
    temp_input = fopen(TEMP_PATH, "w+"); \
    fprintf(temp_input, "%d", input);    \
    fclose(temp_input);                  


int runProgram(const char* programPath);
bool runTest(const char* program1Path, const char* program2Path);

