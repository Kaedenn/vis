
#include "random.h"
#include "helper.h"

#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/* implementation: Lehmer PRNG */
static const uint64_t COEFFICIENT = 279470273UL;
static const uint64_t MODULUS = 4294967291UL;
static uint64_t random_next(uint64_t current) {
    return ((uint64_t)current * COEFFICIENT) % MODULUS;
}

struct random {
    uint64_t seed;
    uint64_t current;
};

prng* random_new(int seed) {
    prng* rng = DBMALLOC(sizeof(struct random));
    rng->seed = rng->current = (uint64_t)seed;
    return rng;
}

void random_free(prng* rng) {
    DZFREE(rng);
}

int random_reseed(prng* rng, int newseed) {
    int oldseed = (int)rng->seed;
    rng->seed = (uint64_t)newseed;
    return oldseed;
}

double random_unit(prng* rng) {
    rng->current = random_next(rng->current);
    return (double)rng->current / (double)MODULUS;
}

double random_range(prng* rng, double low, double high) {
    double unit = (double)random_unit(rng);
    return unit * (high - low) + low;
}

int random_range_int(prng* rng, int low, int high) {
    rng->current = random_next(rng->current);
    return ((int)(rng->current >> 32) * (high - low) + low);
}

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
