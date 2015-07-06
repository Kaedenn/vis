
#include "plimits.h"

void no_limit(UNUSED_PARAM(struct particle* particle)) {
    /* nothing */
}

void box(struct particle* particle) {
    if (particle->x < BOX_FUDGE(particle)) {
        particle->x = BOX_FUDGE(particle);
        particle->dx = 0.0;
    } else if (particle->x > VIS_WIDTH - BOX_FUDGE(particle)) {
        particle->x = VIS_WIDTH - BOX_FUDGE(particle);
        particle->dx = 0.0;
    }
    if (particle->y < BOX_FUDGE(particle)) {
        particle->y = BOX_FUDGE(particle);
        particle->dy = 0.0;
    } else if (particle->y > VIS_HEIGHT - BOX_FUDGE(particle)) {
        particle->y = VIS_HEIGHT - BOX_FUDGE(particle);
        particle->dy = 0.0;
    }
}

void springbox(struct particle* particle) {
    if (particle->x < BOX_FUDGE(particle)) {
        particle->x = BOX_FUDGE(particle);
        particle->dx = -particle->dx;
    } else if (particle->x > VIS_WIDTH - BOX_FUDGE(particle)) {
        particle->x = VIS_WIDTH - BOX_FUDGE(particle);
        particle->dx = -particle->dx;
    }
    if (particle->y < BOX_FUDGE(particle)) {
        particle->y = BOX_FUDGE(particle);
        particle->dy = -particle->dy;
    } else if (particle->y > VIS_HEIGHT - BOX_FUDGE(particle)) {
        particle->y = VIS_HEIGHT - BOX_FUDGE(particle);
        particle->dy = -particle->dy;
    }
}

