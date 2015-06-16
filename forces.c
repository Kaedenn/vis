
#include "forces.h"
#include "helper.h"

#include <math.h>

void no_force(UNUSED_PARAM(particle_t particle)) {
    /* nothing */
}

void friction(particle_t particle) {
    particle->dx *= VIS_FRICTION_COEFF;
    particle->dy *= VIS_FRICTION_COEFF;
}

void gravity(particle_t particle) {
    particle->dy += VIS_GRAVITY_FACTOR;
}
