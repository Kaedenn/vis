
#include "particle.h"

#include "clargs.h"
#include "forces.h"
#include "plimits.h"
#include "helper.h"
#include "particle_extra.h"

#include "random.h"

#include <stdlib.h>
#include <math.h>

particle_t particle_new(double x, double y, double r, int life, void* extra) {
    particle_t p = DBMALLOC(sizeof(struct particle));
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

particle_t particle_new_full(double x, double y,
                             double ux, double uy,
                             double r, double ur,
                             double ds, double uds,
                             double theta, double utheta,
                             int life, int ulife,
                             force_id force, limit_id limit,
                             void* extra) {
    particle_t p = DBMALLOC(sizeof(struct particle));
    
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

void particle_free(particle_t p) {
    free_particle_extra(p->extra);
    DBFREE(p);
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

void particle_tick(particle_t p) {
    p->x += p->dx;
    p->y += p->dy;
    /*switch (p->force) {
        case VIS_FORCE_FRICTION:
            friction(p);
            break;
        case VIS_FORCE_GRAVITY:
            gravity(p);
            break;
        case VIS_DEFAULT_FORCE:
        case VIS_NFORCES:
        default:
            break;
    }
    switch (p->limit) {
        case VIS_LIMIT_BOX:
            box(p);
            break;
        case VIS_LIMIT_SPRINGBOX:
            springbox(p);
            break;
        case VIS_DEFAULT_LIMIT:
        case VIS_NLIMITS:
        default:
            break;
    }*/
    FORCE_MAP[p->force](p);
    LIMIT_MAP[p->limit](p);
    p->life -= 1;
}

inline int particle_is_alive(particle_t p) { return p->life > 0; }
inline double particle_get_x(particle_t p) { return p->x; }
inline double particle_get_y(particle_t p) { return p->y; }
inline double particle_get_dx(particle_t p) { return p->dx; }
inline double particle_get_dy(particle_t p) { return p->dy; }
inline double particle_get_radius(particle_t p) { return p->radius; }
inline int particle_get_life(particle_t p) { return p->life; }
inline int particle_get_lifetime(particle_t p) { return p->lifetime; }
inline void* particle_get_extra(particle_t p) { return p->extra; }

