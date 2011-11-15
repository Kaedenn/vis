
#ifndef VIS_MUTATOR_HEADER_INCLUDED_
#define VIS_MUTATOR_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"

void mutate_push(struct particle* p, double force);
void mutate_slow(struct particle* p, double force);
void mutate_shrink(struct particle* p, double factor);
void mutate_grow(struct particle* p, double factor);

#endif

