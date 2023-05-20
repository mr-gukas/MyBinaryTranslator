#include <stdio.h>
#include <stdlib.h>
#include <chrono>

int main(void)
{
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < 100; ++i)
	    system("res/gukas.elf");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    printf ("Время выполнения, мс: %lu\n", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count());

    return 0;
}
