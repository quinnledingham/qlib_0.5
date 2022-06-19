#ifndef RANDOM_H
#define RANDOM_H

#include <stdlib.h>
#include <ctime>

internal int add = 0;

int Random(int Low, int High)
{
    time_t t;
    srand((unsigned)time(&t) + add++);
    return (rand() % (High - Low + 1) + Low);
}

#endif //RANDOM_H
