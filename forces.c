
#include "forces.h"
#include "helper.h"

#include <math.h>

void no_force(UNUSED_PARAM(particle* p)) {
    /* nothing */
}

void friction(particle* p) {
    p->dx *= VIS_FORCE_FRICTION_COEFF;
    p->dy *= VIS_FORCE_FRICTION_COEFF;
}

void gravity(particle* p) {
    p->dy += VIS_FORCE_GRAVITY_FACTOR;
}
