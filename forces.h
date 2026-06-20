
#ifndef VIS_FORCES_HEADER_INCLUDED_
#define VIS_FORCES_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"

/* default */
static inline void no_force(UNUSED_PARAM(particle_t p)) {}

/* frictional force */
static inline void friction(particle_t p) {
    p->dx *= p->friction_coeff;
    p->dy *= p->friction_coeff;
}

/* gravity */
static inline void gravity(particle_t p) {
    p->dy += p->gravity_coeff;
}

#endif

