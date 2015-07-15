
#ifndef VIS_LIMITS_HEADER_INCLUDED_
#define VIS_LIMITS_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"

/* default */
void no_limit(particle* p);

/* particles stop at borders */
void box(particle* p);

/* particles bounce off borders */
void springbox(particle* p);

static const limit_fn LIMIT_MAP[] = {
    no_limit, box, springbox
};

#endif

