
#include "particle.h"

#include "clargs.h"
#include "forces.h"
#include "plimits.h"
#include "helper.h"
#include "pextra.h"

#include "random.h"

#include <stdlib.h>
#include <math.h>

particle* particle_new(double x, double y, double r, int life, void* extra) {
    particle* p = DBMALLOC(sizeof(particle));
    p->x = x;
    p->y = y;
    p->dx = 0.0;
    p->dy = 0.0;
    p->radius = r;
    p->force = VIS_DEFAULT_FORCE;
    p->limit = VIS_DEFAULT_LIMIT;
    p->lifetime = life;
    p->life = life;
    p->extra = extra;
    return p;
}

particle* particle_new_full(double x, double y,
                            double ux, double uy,
                            double r, double ur,
                            double ds, double uds,
                            double theta, double utheta,
                            int life, int ulife,
                            force_id force, limit_id limit,
                            void* extra) {
    particle* p = DBMALLOC(sizeof(particle));
    ZEROINIT(p);
    
    double t = randdouble(theta-utheta, theta+utheta);
    ds = randdouble(ds-uds, ds+uds);
    
    p->x = randdouble(x-ux, x+ux);
    p->y = randdouble(y-uy, y+uy);
    p->radius = (int)randdouble(r-ur, r+ur);
    p->dx = ds*cos(t);
    p->dy = ds*sin(t);
    
    p->force = force;
    p->limit = limit;
    
    p->lifetime = randint(life-ulife, life+ulife);
    p->life = p->lifetime;
    
    p->extra = extra;
    
    return p;
}

particle* particle_new_circle(double x, double y,
                              double ux, double uy,
                              double s, double us,
                              double r, double ur,
                              double ds, double uds,
                              double theta, double utheta,
                              int life, int ulife,
                              force_id force, limit_id limit, void* extra) {
    particle* p = DBMALLOC(sizeof(particle));
    ZEROINIT(p);

    double angle = randdouble(theta-utheta, theta+utheta);
    double offset = randdouble(s-us, s+us);
    p->x = randdouble(x-ux, x+ux) + offset * cos(angle);
    p->y = randdouble(y-uy, y+uy) + offset * sin(angle);
    double speed = randdouble(ds-uds, ds+uds);
    p->dx = speed * cos(angle);
    p->dy = speed * sin(angle);
    p->radius = (int)randdouble(r-ur, r+ur);
    p->force = force;
    p->limit = limit;
    p->lifetime = randint(life-ulife, life+ulife);
    p->life = p->lifetime;
    p->extra = extra;
    return p;
}

void particle_free(particle* p) {
    free_pextra(p->extra);
    DBFREE(p);
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
inline int particle_is_alive(particle* p) { return p->life > 0; }
inline double particle_get_x(particle* p) { return p->x; }
inline double particle_get_y(particle* p) { return p->y; }
inline double particle_get_dx(particle* p) { return p->dx; }
inline double particle_get_dy(particle* p) { return p->dy; }
inline double particle_get_radius(particle* p) { return p->radius; }
inline int particle_get_life(particle* p) { return p->life; }
inline int particle_get_lifetime(particle* p) { return p->lifetime; }
inline void* particle_get_extra(particle* p) { return p->extra; }
#endif

