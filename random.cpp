#include <stdlib.h>
//#include <time.h>
#include <ctime>

internal int add = 0;

int Random(int Low, int High)
{
    time_t t;
    srand((unsigned)time(&t) + add++);
    
    return (rand() % (High - Low + 1) + Low);
}