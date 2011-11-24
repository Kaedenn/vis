
#include "limits.h"

#define BOX_FUDGE(p) (0)

void no_limit(UNUSED_PARAM(particle_t particle)) {
  /* nothing */
}

void box(particle_t particle) {
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

void springbox(particle_t particle) {
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

