#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "ERROR: expected the name of the file to be processed\n");
        return 1;
    }
    
    char front_arg[100] = {};
    sprintf(front_arg, "run/front %s", argv[1]);
    
    system(front_arg);
	system("run/middle");

    return 0;
}

