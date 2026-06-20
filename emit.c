
#include "helper.h"
#include "emit.h"

emit_desc* emit_new(void) {
    return DBMALLOC(sizeof(struct emit));
}

void emit_free(emit_desc* emit) {
    DZFREE(emit);
}

void emit_set_n(emit_desc* e, int n) {
    e->n = n;
}

void emit_set_pos(emit_desc* e, double x, double y, double ux, double uy) {
    e->x = x;
    e->y = y;
    e->ux = ux;
    e->uy = uy;
}

void emit_set_rad_pos(emit_desc* e, double s, double us) {
    e->s = s;
    e->us = us;
}

void emit_set_rad(emit_desc* e, double rad, double urad) {
    e->rad = rad;
    e->urad = urad;
}

void emit_set_depth(emit_desc* e, float depth) {
    e->depth = depth;
}

void emit_set_life(emit_desc* e, int life, int ulife) {
    e->life = life;
    e->ulife = ulife;
}

void emit_set_ds(emit_desc* e, double ds, double uds) {
    e->ds = ds;
    e->uds = uds;
}

void emit_set_vel(emit_desc* e, double dx, double dy) {
    e->dx = dx;
    e->dy = dy;
}

void emit_set_angle(emit_desc* e, double theta, double utheta) {
    e->theta = theta;
    e->utheta = utheta;
}

void emit_set_color(emit_desc* e, float r, float g, float b, float ur, float ug, float ub) {
    e->r = r;
    e->g = g;
    e->b = b;
    e->ur = ur;
    e->ug = ug;
    e->ub = ub;
}

void emit_set_force(emit_desc* e, force_id force) {
    e->force = force;
}

void emit_set_limit(emit_desc* e, limit_id limit) {
    e->limit = limit;
}

void emit_set_blender(emit_desc* e, blend_id blender) {
    e->blender = blender;
}

void emit_set_shape(emit_desc* e, int vertices, float angle) {
    e->vertices = vertices;
    e->angle = angle;
}

void emit_set_tag(emit_desc* e, uint64_t tag) {
    e->tag.ul = tag;
}

void emit_set_friction(emit_desc* e, double friction_coeff) {
    e->friction_coeff = friction_coeff;
}

void emit_set_gravity(emit_desc* e, double gravity_coeff) {
    e->gravity_coeff = gravity_coeff;
}

void dbprintf_emit_desc(emit_desc* e) {
    DBPRINTF("struct emit_desc { n=%d, x=%g, y=%g, ux=%g, uy=%g, "
             "s=%g us=%g, rad=%g, urad=%g, depth=%g, life=%d, ulife=%d, "
             "ds=%g, uds=%g, theta=%g, utheta=%g, r=%f, g=%f, b=%f, "
             "ur=%f, ug=%f, ub=%f, force=%d, limit=%d, blender=%d, "
             "vertices=%d, angle=%f, friction_coeff=%g, gravity_coeff=%g };",
        e->n, e->x, e->y, e->ux, e->uy, e->s, e->us, e->rad, e->urad,
        e->depth, e->life, e->ulife, e->ds, e->uds, e->theta, e->utheta,
        e->r, e->g, e->b, e->ur, e->ug, e->ub, e->force, e->limit,
        e->blender, e->vertices, e->angle, e->friction_coeff, e->gravity_coeff);
}
