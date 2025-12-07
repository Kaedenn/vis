
#ifndef VIS_PARTICLE_HEADER_INCLUDED_
#define VIS_PARTICLE_HEADER_INCLUDED_ 1

#include "defines.h"
#include "types.h"
#include <stdlib.h>

struct particle;

typedef void (*force_fn)(struct particle* p);
typedef void (*limit_fn)(struct particle* p);

typedef struct particle {
    double x, y;
    double dx, dy;
    double radius;
    int lifetime;
    int life;
    force_id force;
    limit_id limit;
    void* extra;
} particle;

/* constructors */
particle* particle_new(double x, double y, double r, int life, void* extra);
particle* particle_new_full(double x, double y, double ux, double uy, double r,
                            double ur, double ds, double uds, double theta,
                            double utheta, int life, int ulife, force_id force,
                            limit_id limit, void* extra);

/* destructor */
void particle_free(particle* p);

/* specific mutation functions */
void particle_push(particle* p, double dx, double dy);
void particle_set_force(particle* p, force_id force);
void particle_set_limit(particle* p, limit_id limit);

/* special function for a particle's life to continue */
void particle_tick(particle* p);

/* accessor functions */
int particle_is_alive(particle* p);
double particle_get_x(particle* p);
double particle_get_y(particle* p);
double particle_get_dx(particle* p);
double particle_get_dy(particle* p);
double particle_get_radius(particle* p);
int particle_get_lifetime(particle* p);
int particle_get_life(particle* p);
void* particle_get_extra(particle* p);

#endif
