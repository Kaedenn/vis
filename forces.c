
#include "forces.h"
#include "helper.h"

#include <math.h>

void no_force(UNUSED_PARAM(struct particle* particle)) {
    /* nothing */
}

void friction(struct particle* particle) {
    particle->dx *= VIS_FORCE_FRICTION_COEFF;
    particle->dy *= VIS_FORCE_FRICTION_COEFF;
}

void gravity(struct particle* particle) {
    particle->dy += VIS_FORCE_GRAVITY_FACTOR;
}
