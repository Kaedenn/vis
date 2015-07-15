
#ifndef VIS_MUTATOR_HEADER_INCLUDED_
#define VIS_MUTATOR_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"
#include <stdint.h>

uint64_t mutate_debug_get_particles_mutated(void);
uint64_t mutate_debug_get_particle_tags_modified(void);

struct mutate_method;

typedef void(*mutate_fn)(struct particle* p, struct mutate_method* method);

typedef struct mutate_method {
    mutate_fn func;
    double factor;
    mutate_cond_id cond;
    union particle_tag tag;
} *mutate_method_t;

BOOL mutate_cond_apply(struct particle* p, mutate_method_t method);

void mutate_push(struct particle* p, mutate_method_t method);
void mutate_push_dx(struct particle* p, mutate_method_t method);
void mutate_push_dy(struct particle* p, mutate_method_t method);
void mutate_slow(struct particle* p, mutate_method_t method);
void mutate_shrink(struct particle* p, mutate_method_t method);
void mutate_grow(struct particle* p, mutate_method_t method);
void mutate_age(struct particle* p, mutate_method_t method);
void mutate_opacity(struct particle* p, mutate_method_t method);

void mutate_tag_set(struct particle* p, mutate_method_t method);
void mutate_tag_inc(struct particle* p, mutate_method_t method);
void mutate_tag_dec(struct particle* p, mutate_method_t method);
void mutate_tag_add(struct particle* p, mutate_method_t method);
void mutate_tag_sub(struct particle* p, mutate_method_t method);
void mutate_tag_mul(struct particle* p, mutate_method_t method);
void mutate_tag_div(struct particle* p, mutate_method_t method);

void mutate_push_if(struct particle* p, mutate_method_t method);
void mutate_push_dx_if(struct particle* p, mutate_method_t method);
void mutate_push_dy_if(struct particle* p, mutate_method_t method);
void mutate_slow_if(struct particle* p, mutate_method_t method);
void mutate_shrink_if(struct particle* p, mutate_method_t method);
void mutate_grow_if(struct particle* p, mutate_method_t method);
void mutate_age_if(struct particle* p, mutate_method_t method);
void mutate_opacity_if(struct particle* p, mutate_method_t method);

static const mutate_fn MUTATE_MAP[] = {
    mutate_push,
    mutate_push_dx,
    mutate_push_dy,
    mutate_slow,
    mutate_shrink,
    mutate_grow,
    mutate_age,
    mutate_opacity,
    mutate_tag_set,
    mutate_tag_inc,
    mutate_tag_dec,
    mutate_tag_add,
    mutate_tag_sub,
    mutate_tag_mul,
    mutate_tag_div,
    mutate_push_if,
    mutate_push_dx_if,
    mutate_push_dy_if,
    mutate_slow_if,
    mutate_shrink_if,
    mutate_grow_if,
    mutate_age_if,
    mutate_opacity_if,
};

#endif

