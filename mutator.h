
#ifndef VIS_MUTATOR_HEADER_INCLUDED_
#define VIS_MUTATOR_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"
#include <stdint.h>

uint64_t mutate_debug_get_particles_mutated(void);
uint64_t mutate_debug_get_particle_tags_modified(void);

struct mutate_method;

typedef void(*mutate_fn)(particle_t p, struct mutate_method* method);

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

BOOL mutate_cond_apply(particle_t p, mutate_method* method);

void mutate_push(particle_t p, mutate_method* method);
void mutate_push_dx(particle_t p, mutate_method* method);
void mutate_push_dy(particle_t p, mutate_method* method);
void mutate_push_dz(particle_t p, mutate_method* method);
void mutate_slow(particle_t p, mutate_method* method);
void mutate_shrink(particle_t p, mutate_method* method);
void mutate_grow(particle_t p, mutate_method* method);
void mutate_age(particle_t p, mutate_method* method);
void mutate_opacity(particle_t p, mutate_method* method);

void mutate_set_dx(particle_t p, mutate_method* method);
void mutate_set_dy(particle_t p, mutate_method* method);
void mutate_set_dz(particle_t p, mutate_method* method);
void mutate_set_radius(particle_t p, mutate_method* method);
void mutate_set_vertices(particle_t p, mutate_method* method);
void mutate_set_angle(particle_t p, mutate_method* method);
void mutate_set_friction(particle_t p, mutate_method* method);
void mutate_set_gravity(particle_t p, mutate_method* method);

void mutate_tag_set(particle_t p, mutate_method* method);
void mutate_tag_inc(particle_t p, mutate_method* method);
void mutate_tag_dec(particle_t p, mutate_method* method);
void mutate_tag_add(particle_t p, mutate_method* method);
void mutate_tag_sub(particle_t p, mutate_method* method);
void mutate_tag_mul(particle_t p, mutate_method* method);
void mutate_tag_div(particle_t p, mutate_method* method);
void mutate_tag_set_if(particle_t p, mutate_method* method);

void mutate_push_if(particle_t p, mutate_method* method);
void mutate_push_dx_if(particle_t p, mutate_method* method);
void mutate_push_dy_if(particle_t p, mutate_method* method);
void mutate_push_dz_if(particle_t p, mutate_method* method);
void mutate_slow_if(particle_t p, mutate_method* method);
void mutate_shrink_if(particle_t p, mutate_method* method);
void mutate_grow_if(particle_t p, mutate_method* method);
void mutate_age_if(particle_t p, mutate_method* method);
void mutate_opacity_if(particle_t p, mutate_method* method);

void mutate_set_dx_if(particle_t p, mutate_method* method);
void mutate_set_dy_if(particle_t p, mutate_method* method);
void mutate_set_dz_if(particle_t p, mutate_method* method);
void mutate_set_radius_if(particle_t p, mutate_method* method);
void mutate_set_vertices_if(particle_t p, mutate_method* method);
void mutate_set_angle_if(particle_t p, mutate_method* method);
void mutate_set_friction_if(particle_t p, mutate_method* method);
void mutate_set_gravity_if(particle_t p, mutate_method* method);

void mutate_none(particle_t p, mutate_method* method);

/* Must match the mutate_id enum in types.h */
static const mutate_fn MUTATE_MAP[VIS_NMUTATES+1] = {
    [VIS_MUTATE_PUSH] = mutate_push,
    [VIS_MUTATE_PUSH_DX] = mutate_push_dx,
    [VIS_MUTATE_PUSH_DY] = mutate_push_dy,
    [VIS_MUTATE_PUSH_DZ] = mutate_push_dz,
    [VIS_MUTATE_SLOW] = mutate_slow,
    [VIS_MUTATE_SHRINK] = mutate_shrink,
    [VIS_MUTATE_GROW] = mutate_grow,
    [VIS_MUTATE_AGE] = mutate_age,
    [VIS_MUTATE_OPACITY] = mutate_opacity,
    [VIS_MUTATE_SET_DX] = mutate_set_dx,
    [VIS_MUTATE_SET_DY] = mutate_set_dy,
    [VIS_MUTATE_SET_DZ] = mutate_set_dz,
    [VIS_MUTATE_SET_RADIUS] = mutate_set_radius,
    [VIS_MUTATE_SET_VERTICES] = mutate_set_vertices,
    [VIS_MUTATE_SET_ANGLE] = mutate_set_angle,
    [VIS_MUTATE_SET_FRICTION] = mutate_set_friction,
    [VIS_MUTATE_SET_GRAVITY] = mutate_set_gravity,
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
    [VIS_MUTATE_PUSH_DZ_IF] = mutate_push_dz_if,
    [VIS_MUTATE_SLOW_IF] = mutate_slow_if,
    [VIS_MUTATE_SHRINK_IF] = mutate_shrink_if,
    [VIS_MUTATE_GROW_IF] = mutate_grow_if,
    [VIS_MUTATE_AGE_IF] = mutate_age_if,
    [VIS_MUTATE_OPACITY_IF] = mutate_opacity_if,
    [VIS_MUTATE_SET_DX_IF] = mutate_set_dx_if,
    [VIS_MUTATE_SET_DY_IF] = mutate_set_dy_if,
    [VIS_MUTATE_SET_DZ_IF] = mutate_set_dz_if,
    [VIS_MUTATE_SET_RADIUS_IF] = mutate_set_radius_if,
    [VIS_MUTATE_SET_VERTICES_IF] = mutate_set_vertices_if,
    [VIS_MUTATE_SET_ANGLE_IF] = mutate_set_angle_if,
    [VIS_MUTATE_SET_FRICTION_IF] = mutate_set_friction_if,
    [VIS_MUTATE_SET_GRAVITY_IF] = mutate_set_gravity_if,
    [VIS_NMUTATES] = mutate_none
};

static inline BOOL mutate_is_conditional(mutate_id id) {
    switch ((int)id) {
    case VIS_MUTATE_TAG_SET_IF:
    case VIS_MUTATE_PUSH_IF:
    case VIS_MUTATE_PUSH_DX_IF:
    case VIS_MUTATE_PUSH_DY_IF:
    case VIS_MUTATE_PUSH_DZ_IF:
    case VIS_MUTATE_SLOW_IF:
    case VIS_MUTATE_SHRINK_IF:
    case VIS_MUTATE_GROW_IF:
    case VIS_MUTATE_AGE_IF:
    case VIS_MUTATE_OPACITY_IF:
    case VIS_MUTATE_SET_DX_IF:
    case VIS_MUTATE_SET_DY_IF:
    case VIS_MUTATE_SET_DZ_IF:
    case VIS_MUTATE_SET_RADIUS_IF:
    case VIS_MUTATE_SET_VERTICES_IF:
    case VIS_MUTATE_SET_ANGLE_IF:
    case VIS_MUTATE_SET_FRICTION_IF:
    case VIS_MUTATE_SET_GRAVITY_IF:
        return TRUE;
    default:
        return FALSE;
    }
}

static inline BOOL mutate_is_tag(mutate_id id) {
    switch ((int)id) {
    case VIS_MUTATE_TAG_SET:
    case VIS_MUTATE_TAG_INC:
    case VIS_MUTATE_TAG_DEC:
    case VIS_MUTATE_TAG_ADD:
    case VIS_MUTATE_TAG_SUB:
    case VIS_MUTATE_TAG_MUL:
    case VIS_MUTATE_TAG_DIV:
        return TRUE;
    default:
        return FALSE;
    }
}

static inline BOOL mutate_is_unconditional(mutate_id id) {
    switch ((int)id) {
    case VIS_MUTATE_PUSH:
    case VIS_MUTATE_PUSH_DX:
    case VIS_MUTATE_PUSH_DY:
    case VIS_MUTATE_PUSH_DZ:
    case VIS_MUTATE_SLOW:
    case VIS_MUTATE_SHRINK:
    case VIS_MUTATE_GROW:
    case VIS_MUTATE_AGE:
    case VIS_MUTATE_OPACITY:
    case VIS_MUTATE_SET_DX:
    case VIS_MUTATE_SET_DY:
    case VIS_MUTATE_SET_DZ:
    case VIS_MUTATE_SET_RADIUS:
    case VIS_MUTATE_SET_VERTICES:
    case VIS_MUTATE_SET_ANGLE:
    case VIS_MUTATE_SET_FRICTION:
    case VIS_MUTATE_SET_GRAVITY:
        return TRUE;
    default:
        return FALSE;
    }
}

#endif
