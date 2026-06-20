
#include "random.h"
#include "helper.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void seed(void) {
    srand((unsigned)time(NULL));
}

double uniform(double low, double high) {
    return (double)rand() / RAND_MAX * (high - low) + low;
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
