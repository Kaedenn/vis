
#ifndef VIS_MUTATOR_HEADER_INCLUDED_
#define VIS_MUTATOR_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"

typedef struct mutate_method {
    mutate_fn func;
    double factor;
} *mutate_method_t;

/* multiply a particle's velocity by a factor */
void mutate_push(struct particle* p, double factor);

/* multiply a particle's velocity by a factor */
void mutate_push_dx(struct particle* p, double factor);

/* multiply a particle's velocity by a factor */
void mutate_push_dy(struct particle* p, double factor);

/* multiply a particle's velocity by a percent */
void mutate_slow(struct particle* p, double percent);

/* shrink the particle's size by a factor */
void mutate_shrink(struct particle* p, double factor);

/* grow the particle's size by a factor */
void mutate_grow(struct particle* p, double factor);

/* darken a particle's age */
void mutate_age(struct particle* p, double factor);

static const mutate_fn MUTATE_MAP[] = {
    mutate_push,
    mutate_push_dx,
    mutate_push_dy,
    mutate_slow,
    mutate_shrink,
    mutate_grow,
    mutate_age
};

#endif

