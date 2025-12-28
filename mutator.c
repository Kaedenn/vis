
#include "mutator.h"
#include "pextra.h"
#include "random.h"

/* This hack validates the size of the MUTATE_MAP array at compilation
 * time. If the MUTATE_MAP is correct, fnmap_assert_size is char[1].
 * Otherwise, it's char[-1], which is a compile error. This is
 * essentially a static assertion.
 */
typedef char mutate_map_assert_size[
    (sizeof MUTATE_MAP / sizeof MUTATE_MAP[0]) == VIS_NMUTATES+1 ? 1 : -1
];

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

static int64_t get_tag_l(particle* p) {
    return ((pextra*)p->extra)->tag.i.l;
}

BOOL mutate_cond_apply(particle* p, mutate_method* method) {
    switch (method->cond) {
        case VIS_MUTATE_IF_TRUE: return TRUE;
        case VIS_MUTATE_IF_EQ: return get_tag_l(p) == method->tag.i.l;
        case VIS_MUTATE_IF_NE: return get_tag_l(p) != method->tag.i.l;
        case VIS_MUTATE_IF_LT: return get_tag_l(p) < method->tag.i.l;
        case VIS_MUTATE_IF_LE: return get_tag_l(p) <= method->tag.i.l;
        case VIS_MUTATE_IF_GT: return get_tag_l(p) > method->tag.i.l;
        case VIS_MUTATE_IF_GE: return get_tag_l(p) >= method->tag.i.l;
        case VIS_MUTATE_IF_EVEN: return (get_tag_l(p) % 2) == 0;
        case VIS_MUTATE_IF_ODD: return (get_tag_l(p) % 2) == 1;
    }
    return FALSE;
}

void mutate_push(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->dx = p->dx * method->factor[0];
    p->dy = p->dy * method->factor[0];
}

void mutate_push_dx(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->dx = p->dx * method->factor[0];
}

void mutate_push_dy(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->dy = p->dy * method->factor[0];
}

void mutate_slow(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->dx = p->dx / method->factor[0];
    p->dy = p->dy / method->factor[0];
}

void mutate_shrink(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->radius = p->radius / method->factor[0];
}

void mutate_grow(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->radius = p->radius * method->factor[0];
}

void mutate_age(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->life = (int)(p->lifetime * method->factor[0]);
}

void mutate_opacity(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    ((pextra*)p->extra)->a = (float)method->factor[0];
}

void mutate_set_dx(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->dx = randdouble(method->factor[0] - method->factor[1],
                       method->factor[0] + method->factor[1]);
}

void mutate_set_dy(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->dy = randdouble(method->factor[0] - method->factor[1],
                       method->factor[0] + method->factor[1]);
}

void mutate_set_radius(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particles_mutated += 1);
    p->radius = randdouble(method->factor[0] - method->factor[1],
                           method->factor[0] + method->factor[1]);
}

void mutate_tag_set(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra*)p->extra)->tag.i.l = method->tag.i.l;
}

void mutate_tag_inc(particle* p, UNUSED_PARAM(mutate_method* method)) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra*)p->extra)->tag.i.l += 1;
}

void mutate_tag_dec(particle* p, UNUSED_PARAM(mutate_method* method)) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra*)p->extra)->tag.i.l -= 1;
}

void mutate_tag_add(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra*)p->extra)->tag.i.l += method->tag.i.l;
}

void mutate_tag_sub(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra*)p->extra)->tag.i.l -= method->tag.i.l;
}

void mutate_tag_mul(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra*)p->extra)->tag.i.l *= method->tag.i.l;
}

void mutate_tag_div(particle* p, mutate_method* method) {
    DEBUG_EXPRESSION(dbg_ctr.particle_tags_modified += 1);
    ((pextra*)p->extra)->tag.i.l /= method->tag.i.l;
}

#define GEN_COND_MUTATE_FN(mutate) \
    void (mutate##_if)(particle* p, mutate_method* method) { \
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
GEN_COND_MUTATE_FN(mutate_set_dx)
GEN_COND_MUTATE_FN(mutate_set_dy)
GEN_COND_MUTATE_FN(mutate_set_radius)

#undef GEN_COND_MUTATE_FN

void mutate_none(UNUSED_PARAM(particle* p), UNUSED_PARAM(mutate_method* m)) {
}
