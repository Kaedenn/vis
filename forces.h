
#ifndef VIS_FORCES_HEADER_INCLUDED_
#define VIS_FORCES_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"

/* default */
void no_force(particle_t p);

/* frictional force */
void friction(particle_t p);

/* gravity */
void gravity(particle_t p);

static const force_fn FORCE_MAP[] = {
    no_force, friction, gravity
};


#endif

