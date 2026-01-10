
#include "particle.h"

#include "clargs.h"
#include "forces.h"
#include "plimits.h"
#include "helper.h"
#include "pextra.h"

#include "random.h"

#include <stdlib.h>
#include <math.h>

particle* particle_new(double x, double y, double r, int life, pextra* extra) {
    particle* p = DBMALLOC(sizeof(particle));
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
    p->extra = extra;
    return p;
}

particle* particle_new_full(double x, double y,
                            double ux, double uy,
                            double s, double us,
                            double r, double ur,
                            double ds, double uds,
                            double theta, double utheta,
                            int depth, int life, int ulife,
                            force_id force, limit_id limit, pextra* extra)
{
    particle* p = DBMALLOC(sizeof(particle));

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
    p->extra = extra;
    return p;
}

void particle_free(particle* p) {
    free_pextra(p->extra);
    DZFREE(p);
}

void particle_push(particle* p, double dx, double dy) {
    p->dx += dx;
    p->dy += dy;
}

void particle_set_force(particle* p, force_id force) {
    p->force = force;
}

void particle_set_limit(particle* p, limit_id limit) {
    p->limit = limit;
}

void particle_set_color(particle* p, float r, float g, float b, float a) {
    if (p->extra) {
        if (r >= 0) p->extra->r = clampf(r, 0, 1);
        if (g >= 0) p->extra->g = clampf(g, 0, 1);
        if (b >= 0) p->extra->b = clampf(b, 0, 1);
        if (a >= 0) p->extra->a = clampf(a, 0, 1);
    } else {
        DBPRINTF("Ignoring particle_set_color; no pextra on particle");
    }
}

void particle_tick(particle* p) {
    p->x += p->dx;
    p->y += p->dy;
    VIS_ASSERT(0 <= p->force && p->force < VIS_NFORCES);
    VIS_ASSERT(0 <= p->limit && p->limit < VIS_NLIMITS);
    FORCE_MAP[p->force](p);
    LIMIT_MAP[p->limit](p);
    p->life -= 1;
}

#if DEBUG >= DEBUG_NONE
int particle_is_alive(particle* p) { return p->life > 0; }
double particle_get_x(particle* p) { return p->x; }
double particle_get_y(particle* p) { return p->y; }
double particle_get_dx(particle* p) { return p->dx; }
double particle_get_dy(particle* p) { return p->dy; }
double particle_get_radius(particle* p) { return p->radius; }
int particle_get_depth(particle* p) { return p->depth; }
int particle_get_life(particle* p) { return p->life; }
int particle_get_lifetime(particle* p) { return p->lifetime; }
pextra* particle_get_extra(particle* p) { return p->extra; }
#endif

