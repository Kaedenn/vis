
#ifndef VIS_FORCES_HEADER_INCLUDED_
#define VIS_FORCES_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"

/* default */
void no_force(particle* p);

/* frictional force */
void friction(particle* p);

/* gravity */
void gravity(particle* p);

static const force_fn FORCE_MAP[] = {
    no_force, friction, gravity
};


#endif

