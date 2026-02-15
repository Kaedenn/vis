
#include "particle.h"

#include "clargs.h"
#include "forces.h"
#include "plimits.h"
#include "helper.h"

#include "random.h"

#include <math.h>

particle_t particle_new(double x, double y, double r, int life, pextra* extra) {
    particle_t p = DBMALLOC(sizeof(struct particle));
    p->x = x;
    p->y = y;
    p->dx = 0.0;
    p->dy = 0.0;
    p->radius = r;
    p->depth = 0;
    p->force = VIS_DEFAULT_FORCE;
    p->limit = VIS_DEFAULT_LIMIT;
    p->lifetime = life;
    p->life = life;
    if (extra) {
        p->r = extra->r;
        p->g = extra->g;
        p->b = extra->b;
        p->blender = extra->blender;
        p->tag = extra->tag;
        free_pextra(extra);
    }
    return p;
}

particle_t particle_new_full(double x, double y,
                            double ux, double uy,
                            double s, double us,
                            double r, double ur,
                            double ds, double uds,
                            double theta, double utheta,
                            int depth, int life, int ulife,
                            force_id force, limit_id limit,
                            float rgba[4], blend_id blender,
                            union particle_tag tag)
{
    particle_t p = DBMALLOC(sizeof(struct particle));

    double angle = randdouble(theta-utheta, theta+utheta);
    double offset = randdouble(s-us, s+us);
    p->x = randdouble(x-ux, x+ux) + offset * cos(angle);
    p->y = randdouble(y-uy, y+uy) + offset * sin(angle);
    double speed = randdouble(ds-uds, ds+uds);
    p->dx = speed * cos(angle);
    p->dy = speed * sin(angle);
    p->radius = (int)randdouble(r-ur, r+ur);
    p->depth = depth;
    p->force = force;
    p->limit = limit;
    p->lifetime = randint(life-ulife, life+ulife);
    p->life = p->lifetime;
    p->r = rgba[0];
    p->g = rgba[1];
    p->b = rgba[2];
    p->a = rgba[3];
    p->blender = blender;
    p->tag = tag;
    return p;
}

void particle_free(particle_t p) {
    DZFREE(p);
}

void particle_push(particle_t p, double dx, double dy) {
    p->dx += dx;
    p->dy += dy;
}

void particle_set_force(particle_t p, force_id force) {
    p->force = force;
}

void particle_set_limit(particle_t p, limit_id limit) {
    p->limit = limit;
}

void particle_set_color(particle_t p, float r, float g, float b, float a) {
    if (r >= 0) p->r = clampf(r, 0, 1);
    if (g >= 0) p->g = clampf(g, 0, 1);
    if (b >= 0) p->b = clampf(b, 0, 1);
    if (a >= 0) p->a = clampf(a, 0, 1);
}

void particle_tick(particle_t p) {
    p->x += p->dx;
    p->y += p->dy;
    VIS_ASSERT(0 <= p->force && p->force < VIS_NFORCES);
    VIS_ASSERT(0 <= p->limit && p->limit < VIS_NLIMITS);
    FORCE_MAP[p->force](p);
    LIMIT_MAP[p->limit](p);
    p->life -= 1;
}

#if DEBUG >= DEBUG_NONE
int particle_is_alive(particle_t p) { return p->life > 0; }
double particle_get_x(particle_t p) { return p->x; }
double particle_get_y(particle_t p) { return p->y; }
double particle_get_dx(particle_t p) { return p->dx; }
double particle_get_dy(particle_t p) { return p->dy; }
double particle_get_radius(particle_t p) { return p->radius; }
int particle_get_depth(particle_t p) { return p->depth; }
int particle_get_life(particle_t p) { return p->life; }
int particle_get_lifetime(particle_t p) { return p->lifetime; }
#endif

