
#include "mutator.h"
#include "particle_extra.h"

#if DEBUG >= DEBUG_DEBUG
struct mutate_debug_ctr {
    uint64_t particles_mutated;
    uint64_t particle_tags_modified;
};

static struct mutate_debug_ctr dbg_ctr = {0};
#endif

uint64_t mutate_debug_get_particles_mutated(void) {
#if DEBUG >= DEBUG_DEBUG
    return dbg_ctr.particles_mutated;
#else
    return 0;
#endif
}

uint64_t mutate_debug_get_particle_tags_modified(void) {
#if DEBUG >= DEBUG_DEBUG
    return dbg_ctr.particle_tags_modified;
#else
    return 0;
#endif
}

static int64_t get_tag_l(struct particle* p) {
    return ((pextra_t)p->extra)->tag.l;
}

BOOL mutate_cond_apply(struct particle* p, mutate_method_t method) {
    switch (method->cond) {
        case VIS_MUTATE_IF_TRUE: return TRUE;
        case VIS_MUTATE_IF_EQ: return get_tag_l(p) == method->tag.l;
        case VIS_MUTATE_IF_NE: return get_tag_l(p) != method->tag.l;
        case VIS_MUTATE_IF_LT: return get_tag_l(p) < method->tag.l;
        case VIS_MUTATE_IF_LE: return get_tag_l(p) <= method->tag.l;
        case VIS_MUTATE_IF_GT: return get_tag_l(p) > method->tag.l;
        case VIS_MUTATE_IF_GE: return get_tag_l(p) >= method->tag.l;
        case VIS_MUTATE_IF_EVEN: return (get_tag_l(p) % 2) == 0;
        case VIS_MUTATE_IF_ODD: return (get_tag_l(p) % 2) == 1;
    }
    return FALSE;
}

void mutate_push(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->dx = p->dx * method->factor;
    p->dy = p->dy * method->factor;
}

void mutate_push_dx(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->dx = p->dx * method->factor;
}

void mutate_push_dy(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->dy = p->dy * method->factor;
}

void mutate_slow(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->dx = p->dx * method->factor;
    p->dy = p->dy * method->factor;
}

void mutate_shrink(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->radius = p->radius / method->factor;
}

void mutate_grow(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->radius = p->radius * method->factor;
}

void mutate_age(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->life = (int)(p->lifetime * method->factor);
}

void mutate_opacity(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    ((pextra_t)p->extra)->a = (float)method->factor;
}


void mutate_tag_set(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra_t)p->extra)->tag.l = method->tag.l;
}

void mutate_tag_inc(struct particle* p, UNUSED_PARAM(mutate_method_t method)) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra_t)p->extra)->tag.l += 1;
}

void mutate_tag_dec(struct particle* p, UNUSED_PARAM(mutate_method_t method)) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra_t)p->extra)->tag.l -= 1;
}

void mutate_tag_add(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra_t)p->extra)->tag.l += method->tag.l;
}

void mutate_tag_sub(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra_t)p->extra)->tag.l -= method->tag.l;
}

void mutate_tag_mul(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra_t)p->extra)->tag.l *= method->tag.l;
}

void mutate_tag_div(struct particle* p, mutate_method_t method) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra_t)p->extra)->tag.l /= method->tag.l;
}


#define GEN_COND_MUTATE_FN(mutate) \
    void (mutate##_if)(struct particle* p, mutate_method_t method) { \
        if (mutate_cond_apply(p, method)) { \
            (mutate)(p, method); \
        } \
    }

GEN_COND_MUTATE_FN(mutate_push)
GEN_COND_MUTATE_FN(mutate_push_dx)
GEN_COND_MUTATE_FN(mutate_push_dy)
GEN_COND_MUTATE_FN(mutate_slow)
GEN_COND_MUTATE_FN(mutate_shrink)
GEN_COND_MUTATE_FN(mutate_grow)
GEN_COND_MUTATE_FN(mutate_age)
GEN_COND_MUTATE_FN(mutate_opacity)

#undef GEN_COND_MUTATE_FN

