
#ifndef VIS_MUTATOR_HEADER_INCLUDED_
#define VIS_MUTATOR_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"
#include <stdint.h>

uint64_t mutate_debug_get_particles_mutated(void);
uint64_t mutate_debug_get_particle_tags_modified(void);

struct mutate_method;

typedef void(*mutate_fn)(particle* p, struct mutate_method* method);

typedef struct mutate_method {
    mutate_id id;
    mutate_fn func;
    double check_factor[2]; /* for conditional mutates */
    double factor[2];       /* coefficient, amount, etc */
    double offset[2];       /* location (x, y) in world space */
    mutate_cond_id cond;
    union particle_tag tag;     /* for conditional mutates */
    union particle_tag newtag;
} mutate_method;

BOOL mutate_cond_apply(particle* p, mutate_method* method);

void mutate_push(particle* p, mutate_method* method);
void mutate_push_dx(particle* p, mutate_method* method);
void mutate_push_dy(particle* p, mutate_method* method);
void mutate_slow(particle* p, mutate_method* method);
void mutate_shrink(particle* p, mutate_method* method);
void mutate_grow(particle* p, mutate_method* method);
void mutate_age(particle* p, mutate_method* method);
void mutate_opacity(particle* p, mutate_method* method);

void mutate_set_dx(particle* p, mutate_method* method);
void mutate_set_dy(particle* p, mutate_method* method);
void mutate_set_radius(particle* p, mutate_method* method);

void mutate_tag_set(particle* p, mutate_method* method);
void mutate_tag_inc(particle* p, mutate_method* method);
void mutate_tag_dec(particle* p, mutate_method* method);
void mutate_tag_add(particle* p, mutate_method* method);
void mutate_tag_sub(particle* p, mutate_method* method);
void mutate_tag_mul(particle* p, mutate_method* method);
void mutate_tag_div(particle* p, mutate_method* method);
void mutate_tag_set_if(particle* p, mutate_method* method);

void mutate_push_if(particle* p, mutate_method* method);
void mutate_push_dx_if(particle* p, mutate_method* method);
void mutate_push_dy_if(particle* p, mutate_method* method);
void mutate_slow_if(particle* p, mutate_method* method);
void mutate_shrink_if(particle* p, mutate_method* method);
void mutate_grow_if(particle* p, mutate_method* method);
void mutate_age_if(particle* p, mutate_method* method);
void mutate_opacity_if(particle* p, mutate_method* method);

void mutate_set_dx_if(particle* p, mutate_method* method);
void mutate_set_dy_if(particle* p, mutate_method* method);
void mutate_set_radius_if(particle* p, mutate_method* method);

void mutate_none(particle* p, mutate_method* method);

/* Must match the mutate_id enum in types.h */
static const mutate_fn MUTATE_MAP[VIS_NMUTATES+1] = {
    [VIS_MUTATE_PUSH] = mutate_push,
    [VIS_MUTATE_PUSH_DX] = mutate_push_dx,
    [VIS_MUTATE_PUSH_DY] = mutate_push_dy,
    [VIS_MUTATE_SLOW] = mutate_slow,
    [VIS_MUTATE_SHRINK] = mutate_shrink,
    [VIS_MUTATE_GROW] = mutate_grow,
    [VIS_MUTATE_AGE] = mutate_age,
    [VIS_MUTATE_OPACITY] = mutate_opacity,
    [VIS_MUTATE_SET_DX] = mutate_set_dx,
    [VIS_MUTATE_SET_DY] = mutate_set_dy,
    [VIS_MUTATE_SET_RADIUS] = mutate_set_radius,
    [VIS_MUTATE_TAG_SET] = mutate_tag_set,
    [VIS_MUTATE_TAG_INC] = mutate_tag_inc,
    [VIS_MUTATE_TAG_DEC] = mutate_tag_dec,
    [VIS_MUTATE_TAG_ADD] = mutate_tag_add,
    [VIS_MUTATE_TAG_SUB] = mutate_tag_sub,
    [VIS_MUTATE_TAG_MUL] = mutate_tag_mul,
    [VIS_MUTATE_TAG_DIV] = mutate_tag_div,
    [VIS_MUTATE_TAG_SET_IF] = mutate_tag_set_if,
    [VIS_MUTATE_PUSH_IF] = mutate_push_if,
    [VIS_MUTATE_PUSH_DX_IF] = mutate_push_dx_if,
    [VIS_MUTATE_PUSH_DY_IF] = mutate_push_dy_if,
    [VIS_MUTATE_SLOW_IF] = mutate_slow_if,
    [VIS_MUTATE_SHRINK_IF] = mutate_shrink_if,
    [VIS_MUTATE_GROW_IF] = mutate_grow_if,
    [VIS_MUTATE_AGE_IF] = mutate_age_if,
    [VIS_MUTATE_OPACITY_IF] = mutate_opacity_if,
    [VIS_MUTATE_SET_DX_IF] = mutate_set_dx_if,
    [VIS_MUTATE_SET_DY_IF] = mutate_set_dy_if,
    [VIS_MUTATE_SET_RADIUS_IF] = mutate_set_radius_if,
    [VIS_NMUTATES] = mutate_none
};

#endif

