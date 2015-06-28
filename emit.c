
#include <stdlib.h>
#include "defines.h"
#include "emit.h"

/* n x y ux uy rad urad ds uds theta utheta life ulife r g b ur ug ub force limit blender */
emit_t make_emit_frame(int n,
        double x, double y, double ux, double uy,
        double rad, double urad, double ds, double uds,
        double theta, double utheta, int life, int ulife,
        float r, float g, float b, float ur, float ub, float ug,
        force_t force, limit_t limit, blend_t blend) /* sorry */ {
    emit_t emit = emit_new();
    emit->n = n;
    emit->x = x;
    emit->y = y;
    emit->ux = ux;
    emit->uy = uy;
    emit->rad = rad;
    emit->urad = urad;
    emit->life = life;
    emit->ulife = ulife;
    emit->ds = ds;
    emit->uds = uds;
    emit->theta = theta;
    emit->utheta = utheta;
    emit->r = r;
    emit->b = b;
    emit->g = g;
    emit->ur = ur;
    emit->ub = ub;
    emit->ug = ug;
    emit->force = force;
    emit->limit = limit;
    emit->blender = blend;
    return emit;
}

emit_t emit_new(void) { return DBMALLOC(sizeof(struct emit_frame)); }
void emit_free(emit_t emit) { DBFREE(emit); }

void emit_set_n(emit_t e, int n) { e->n = n; }
void emit_set_pos(emit_t e, double x, double y, double ux, double uy) {
    e->x = x;
    e->y = y;
    e->ux = ux;
    e->uy = uy;
}
void emit_set_rad(emit_t e, double rad, double urad) {
    e->rad = rad;
    e->urad = urad;
}
void emit_set_life(emit_t e, int life, int ulife) {
    e->life = life;
    e->ulife = ulife;
}
void emit_set_ds(emit_t e, double ds, double uds) {
    e->ds = ds;
    e->uds = uds;
}
void emit_set_angle(emit_t e, double theta, double utheta) {
    e->theta = theta;
    e->utheta = utheta;
}
void emit_set_color(emit_t e, float r, float g, float b,
                    float ur, float ug, float ub) {
    e->r = r;
    e->g = g;
    e->b = b;
    e->ur = ur;
    e->ug = ug;
    e->ub = ub;
}
void emit_set_force(emit_t e, force_t force) { e->force = force; }
void emit_set_limit(emit_t e, limit_t limit) { e->limit = limit; }
void emit_set_blender(emit_t e, blend_t blender) { e->blender = blender; }

void dbprintf_emit_t(emit_t e) {
    DBPRINTF("struct *emit_t { n=%d, x=%g, y=%g, ux=%g, uy=%g, "
             "rad=%g, urad=%g, life=%d, ulife=%d, ds=%g, uds=%g, "
             "theta=%g, utheta=%g, r=%g, g=%g, b=%g, "
             "ur=%g, ug=%g, ub=%g, force=%d, limit=%d, blender=%d };",
             e->n, e->x, e->y, e->ux, e->uy, e->rad, e->urad,
             e->life, e->ulife, e->ds, e->uds, e->theta, e->utheta,
             (double)e->r, (double)e->g, (double)e->b,
             (double)e->ur, (double)e->ug, (double)e->ub,
             e->force, e->limit, e->blender);
}

