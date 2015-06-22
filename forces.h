
#ifndef VIS_FORCES_HEADER_INCLUDED_
#define VIS_FORCES_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"

/* default */
void no_force(particle_t particle);

/* frictional force */
void friction(particle_t particle);

/* gravity */
void gravity(particle_t particle);

static const force_fn FORCE_MAP[] = {
    no_force, friction, gravity
};


#endif

