
#ifndef VIS_LIMITS_HEADER_INCLUDED_
#define VIS_LIMITS_HEADER_INCLUDED_ 1

#include "defines.h"
#include "particle.h"

extern unsigned int screen_size[2];

void plimits_update_screen_size(unsigned int width, unsigned int height);

/* default */
static inline void no_limit(UNUSED_PARAM(particle_t p)) {}

/* particles stop at borders */
static inline void box(particle_t p) {
    if (p->x < 0) {
        p->x = 0;
        p->dx = 0.0;
    } else if (p->x > screen_size[0] - 0) {
        p->x = screen_size[0] - 0;
        p->dx = 0.0;
    }
    if (p->y < 0) {
        p->y = 0;
        p->dy = 0.0;
    } else if (p->y > screen_size[1] - 0) {
        p->y = screen_size[1] - 0;
        p->dy = 0.0;
    }
}

/* particles bounce off borders */
static inline void springbox(particle_t p) {
    if (p->x < 0) {
        p->x = 0;
        p->dx = -p->dx;
    } else if (p->x > screen_size[0] - 0) {
        p->x = screen_size[0] - 0;
        p->dx = -p->dx;
    }
    if (p->y < 0) {
        p->y = 0;
        p->dy = -p->dy;
    } else if (p->y > screen_size[1] - 0) {
        p->y = screen_size[1] - 0;
        p->dy = -p->dy;
    }
}

#endif

