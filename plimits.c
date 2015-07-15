
#include "plimits.h"

void no_limit(UNUSED_PARAM(particle* p)) {
    /* nothing */
}

void box(particle* p) {
    if (p->x < BOX_FUDGE(p)) {
        p->x = BOX_FUDGE(p);
        p->dx = 0.0;
    } else if (p->x > VIS_WIDTH - BOX_FUDGE(p)) {
        p->x = VIS_WIDTH - BOX_FUDGE(p);
        p->dx = 0.0;
    }
    if (p->y < BOX_FUDGE(p)) {
        p->y = BOX_FUDGE(p);
        p->dy = 0.0;
    } else if (p->y > VIS_HEIGHT - BOX_FUDGE(p)) {
        p->y = VIS_HEIGHT - BOX_FUDGE(p);
        p->dy = 0.0;
    }
}

void springbox(particle* p) {
    if (p->x < BOX_FUDGE(p)) {
        p->x = BOX_FUDGE(p);
        p->dx = -p->dx;
    } else if (p->x > VIS_WIDTH - BOX_FUDGE(p)) {
        p->x = VIS_WIDTH - BOX_FUDGE(p);
        p->dx = -p->dx;
    }
    if (p->y < BOX_FUDGE(p)) {
        p->y = BOX_FUDGE(p);
        p->dy = -p->dy;
    } else if (p->y > VIS_HEIGHT - BOX_FUDGE(p)) {
        p->y = VIS_HEIGHT - BOX_FUDGE(p);
        p->dy = -p->dy;
    }
}

