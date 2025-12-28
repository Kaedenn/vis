
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

/* create a simple particle with default values */
particle* particle_new(double x, double y, double r, int life, void* extra);

/* create a particle with an X/Y position */
particle* particle_new_full(double x, double y, double ux, double uy,
                            double r, double ur, double ds, double uds,
                            double theta, double utheta, int life, int ulife,
                            force_id force, limit_id limit, void* extra);

/* create a particle with both X/Y position and circle offset */
particle* particle_new_circle(double x, double y, double ux, double uy,
                              double s, double us, double r, double ur,
                              double ds, double uds,
                              double theta, double utheta,
                              int life, int ulife,
                              force_id force, limit_id limit, void* extra);

/* destructor */
void particle_free(particle* p);

/* specific mutation functions */
void particle_push(particle* p, double dx, double dy);
void particle_set_force(particle* p, force_id force);
void particle_set_limit(particle* p, limit_id limit);

/* special function for a particle's life to continue */
void particle_tick(particle* p);

#if DEBUG > DEBUG_NONE
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
#else
#define particle_is_alive(p) ((p)->life > 0)
#define particle_get_x(p) ((p)->x)
#define particle_get_y(p) ((p)->y)
#define particle_get_dx(p) ((p)->dx)
#define particle_get_dy(p) ((p)->dy)
#define particle_get_radius(p) ((p)->radius)
#define particle_get_life(p) ((p)->life)
#define particle_get_lifetime(p) ((p)->lifetime)
#define particle_get_extra(p) ((p)->extra)
#endif

#endif
