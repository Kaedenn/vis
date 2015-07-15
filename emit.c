
#include <stdlib.h>
#include "defines.h"
#include "emit.h"

/* n x y ux uy rad urad ds uds theta utheta life ulife r g b ur ug ub
 * force limit blender */
emit_desc make_emit_frame(int n,
        double x, double y, double ux, double uy,
        double rad, double urad, double ds, double uds,
        double theta, double utheta, int life, int ulife,
        float r, float g, float b, float ur, float ub, float ug,
        force_id force, limit_id limit, blend_id blend) /* sorry */ {
    emit_desc emit = emit_new();
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

emit_desc emit_new(void) { return DBMALLOC(sizeof(struct emit)); }
void emit_free(emit_desc emit) { DBFREE(emit); }

void emit_set_n(emit_desc e, int n) { e->n = n; }
void emit_set_pos(emit_desc e, double x, double y, double ux, double uy) {
    e->x = x;
    e->y = y;
    e->ux = ux;
    e->uy = uy;
}
void emit_set_rad(emit_desc e, double rad, double urad) {
    e->rad = rad;
    e->urad = urad;
}
void emit_set_life(emit_desc e, int life, int ulife) {
    e->life = life;
    e->ulife = ulife;
}
void emit_set_ds(emit_desc e, double ds, double uds) {
    e->ds = ds;
    e->uds = uds;
}
void emit_set_angle(emit_desc e, double theta, double utheta) {
    e->theta = theta;
    e->utheta = utheta;
}
void emit_set_color(emit_desc e, float r, float g, float b,
                    float ur, float ug, float ub) {
    e->r = r;
    e->g = g;
    e->b = b;
    e->ur = ur;
    e->ug = ug;
    e->ub = ub;
}
void emit_set_force(emit_desc e, force_id force) { e->force = force; }
void emit_set_limit(emit_desc e, limit_id limit) { e->limit = limit; }
void emit_set_blender(emit_desc e, blend_id blender) { e->blender = blender; }

void dbprintf_emit_desc(emit_desc e) {
    DBPRINTF("struct *emit_desc { n=%d, x=%g, y=%g, ux=%g, uy=%g, "
             "rad=%g, urad=%g, life=%d, ulife=%d, ds=%g, uds=%g, "
             "theta=%g, utheta=%g, r=%g, g=%g, b=%g, "
             "ur=%g, ug=%g, ub=%g, force=%d, limit=%d, blender=%d };",
             e->n, e->x, e->y, e->ux, e->uy, e->rad, e->urad,
             e->life, e->ulife, e->ds, e->uds, e->theta, e->utheta,
             (double)e->r, (double)e->g, (double)e->b,
             (double)e->ur, (double)e->ug, (double)e->ub,
             e->force, e->limit, e->blender);
}

