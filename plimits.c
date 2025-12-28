
#include "plimits.h"

static unsigned int screen_size[2] = {
    VIS_WIDTH,
    VIS_HEIGHT
};

void plimits_update_screen_size(unsigned int width, unsigned int height) {
    screen_size[0] = width;
    screen_size[1] = height;
}

void no_limit(UNUSED_PARAM(particle* p)) {
    /* nothing */
}

void box(particle* p) {
    if (p->x < BOX_FUDGE(p)) {
        p->x = BOX_FUDGE(p);
        p->dx = 0.0;
    } else if (p->x > screen_size[0] - BOX_FUDGE(p)) {
        p->x = screen_size[0] - BOX_FUDGE(p);
        p->dx = 0.0;
    }
    if (p->y < BOX_FUDGE(p)) {
        p->y = BOX_FUDGE(p);
        p->dy = 0.0;
    } else if (p->y > screen_size[1] - BOX_FUDGE(p)) {
        p->y = screen_size[1] - BOX_FUDGE(p);
        p->dy = 0.0;
    }
}

void springbox(particle* p) {
    if (p->x < BOX_FUDGE(p)) {
        p->x = BOX_FUDGE(p);
        p->dx = -p->dx;
    } else if (p->x > screen_size[0] - BOX_FUDGE(p)) {
        p->x = screen_size[0] - BOX_FUDGE(p);
        p->dx = -p->dx;
    }
    if (p->y < BOX_FUDGE(p)) {
        p->y = BOX_FUDGE(p);
        p->dy = -p->dy;
    } else if (p->y > screen_size[1] - BOX_FUDGE(p)) {
        p->y = screen_size[1] - BOX_FUDGE(p);
        p->dy = -p->dy;
    }
}

