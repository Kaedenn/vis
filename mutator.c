
#include "mutator.h"

void mutate_push(struct particle* p, double factor) {
    p->dx = p->dx * factor;
}

void mutate_slow(struct particle* p, double percent) {
    p->dx = p->dx * percent;
}

void mutate_shrink(struct particle* p, double factor) {
    p->radius = p->radius / factor;
}

void mutate_grow(struct particle* p, double factor) {
    p->radius = p->radius * factor;
}

