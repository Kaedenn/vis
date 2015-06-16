
#include "random.h"

#include <stdlib.h>
#include <time.h>

/* TODO: implement a cache of random numbers */

void seed(void) {
    srand((unsigned)time(NULL));
}

double uniform(double low, double high) {
    return (double)rand() / RAND_MAX * (high - low) + low;
}

double triangular(double low, double high) {
    register double v = uniform(low, high);
    /* 2x if left half, -2x+u if right half */
    if (v < (high + low) / 2) {
        return 2*v;
    } else {
        return -2*v + (high + low)/2;
    }
}

double randdouble(double low, double high) {
    return uniform(low, high);
}

float randfloat(float low, float high) {
    return (float)randdouble(low, high);
}

int randint(int low, int high) {
    return (int)randdouble(low, high);
}

