
#ifndef VIS_FORCES_HEADER_INCLUDED_
#define VIS_FORCES_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"

/* default */
void no_force(struct particle* particle);

/* frictional force */
void friction(struct particle* particle);

/* gravity */
void gravity(struct particle* particle);

static const force_fn FORCE_MAP[] = {
    no_force, friction, gravity
};


#endif

