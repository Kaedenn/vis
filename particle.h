
#ifndef VIS_PARTICLE_HEADER_INCLUDED_
#define VIS_PARTICLE_HEADER_INCLUDED_ 1

#include "defines.h"
#include <stdlib.h>

struct particle;

typedef void (*force_fn)(struct particle* p);
typedef void (*limit_fn)(struct particle* p);

struct particle {
    double x, y;
    double dx, dy;
    double radius;
    int lifetime;
    int life;
    force_id force;
    limit_id limit;
    void* extra;
};

/* constructors */
struct particle* particle_new(double x, double y, double r, int life,
                              void* extra);
struct particle* particle_new_full(double x, double y,
                             double ux, double uy,
                             double r, double ur,
                             double ds, double uds,
                             double theta, double utheta,
                             int life, int ulife,
                             force_id force, limit_id limit,
                             void* extra);

/* destructor */
void particle_free(struct particle* p);

/* specific mutation functions */
void particle_push(struct particle* p, double dx, double dy);
void particle_set_force(struct particle* p, force_id force);
void particle_set_limit(struct particle* p, limit_id limit);

/* special function for a particle's life to continue */
void particle_tick(struct particle* p);

/* accessor functions */
int particle_is_alive(struct particle* p);
double particle_get_x(struct particle* p);
double particle_get_y(struct particle* p);
double particle_get_dx(struct particle* p);
double particle_get_dy(struct particle* p);
double particle_get_radius(struct particle* p);
int particle_get_lifetime(struct particle* p);
int particle_get_life(struct particle* p);
void* particle_get_extra(struct particle* p);

#endif
