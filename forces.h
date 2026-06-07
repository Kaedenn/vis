
#ifndef VIS_FORCES_HEADER_INCLUDED_
#define VIS_FORCES_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"

/* default */
static inline void no_force(UNUSED_PARAM(particle_t p)) {}

/* frictional force */
static inline void friction(particle_t p) {
    p->dx *= VIS_FORCE_FRICTION_COEFF;
    p->dy *= VIS_FORCE_FRICTION_COEFF;
}

/* gravity */
static inline void gravity(particle_t p) {
    p->dy += VIS_FORCE_GRAVITY_FACTOR;
}

#endif

