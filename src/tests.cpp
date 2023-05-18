#include "../include/tests.hpp"

int runProgram(const char* programPath) {
    char command[100] = {};
    sprintf(command, "%s < %s", programPath, TEMP_PATH); 

    FILE* fp = popen(command, "r"); 
    if (fp == NULL) {
        printf("Failed to run program: %s\n", programPath);
        exit(1);
    }

    int result = 0;

    fscanf(fp, "%d", &result);
    pclose(fp);

    return result;
}

bool runTest(const char* program1Path, const char* program2Path) {
    int result1 = runProgram(program1Path);
    int result2 = runProgram(program2Path);
    
    return (result1 == result2) ? true : false;
}

int main() {
    const char* x86_path  = PATH_1;
    const char* y86_path  = PATH_2; 
    const char* inputFilePath = INPUT_PATH;

    
    FILE* inputFile  = fopen(inputFilePath, "r");
    FILE* temp_input = NULL;

    if (inputFile == NULL) {
        printf("Failed to open input file: %s\n", inputFilePath);
        return 1;
    }
    
    int index     = 0;
    int input     = 0;
    int testCount = 0;

    while (fscanf(inputFile, "%d", &input) != EOF) {
        temp_input = fopen(TEMP_PATH, "w+");
        fprintf(temp_input, "%d", input);
        fclose(temp_input); 

        ++index;
        bool testPassed = runTest(x86_path, y86_path);

        printf("Test №%d: ", index);
        if (testPassed) {
            printf("\033[0;32mPassed\033[0m\n");
            testCount++;
        }
        else {
            printf("\033[0;31mFailed\033[0m\n");
        }
    }

    fclose(inputFile);

    if (testCount == index) {
        printf("\033[0;32mAll tests passed successfully\033[0m\n\n");
    }

    return 0;
}



