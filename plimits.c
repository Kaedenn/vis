
#include "plimits.h"

static unsigned int screen_size[2] = {
    VIS_WIDTH,
    VIS_HEIGHT
};

void plimits_update_screen_size(unsigned int width, unsigned int height) {
    screen_size[0] = width;
    screen_size[1] = height;
}

void no_limit(UNUSED_PARAM(particle_t p)) {
    /* nothing */
}

void box(particle_t p) {
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

void springbox(particle_t p) {
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

