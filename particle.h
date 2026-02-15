
#ifndef VIS_PARTICLE_HEADER_INCLUDED_
#define VIS_PARTICLE_HEADER_INCLUDED_ 1

#include "defines.h"
#include "pextra.h"
#include "types.h"

struct particle;

typedef void (*force_fn)(struct particle* p);
typedef void (*limit_fn)(struct particle* p);

struct particle {
    double x, y;
    double dx, dy;
    double radius;
    int depth;
    int lifetime;
    int life;
    force_id force;
    limit_id limit;
    float r, g, b, a;
    blend_id blender;
    union particle_tag tag;
};
typedef struct particle* particle_t;

/* create a simple particle with default values */
particle_t particle_new(double x, double y, double r, int life, pextra* extra);

/* create a particle with both X/Y position and circle offset */
particle_t particle_new_full(double x, double y, double ux, double uy,
                            double s, double us, double r, double ur,
                            double ds, double uds,
                            double theta, double utheta,
                            int depth, int life, int ulife,
                            force_id force, limit_id limit,
                            float rgba[4], blend_id blender,
                            union particle_tag tag);

/* destructor */
void particle_free(particle_t p);

/* specific mutation functions */
void particle_push(particle_t p, double dx, double dy);
void particle_set_force(particle_t p, force_id force);
void particle_set_limit(particle_t p, limit_id limit);

/* values less than 0 preserve color; values are clamped to 0..1 */
void particle_set_color(particle_t p, float r, float g, float b, float a);

/* special function for a particle's life to continue */
void particle_tick(particle_t p);

/* accessor functions */
#if DEBUG > DEBUG_NONE
int particle_is_alive(particle_t p);
double particle_get_x(particle_t p);
double particle_get_y(particle_t p);
double particle_get_dx(particle_t p);
double particle_get_dy(particle_t p);
double particle_get_radius(particle_t p);
int particle_get_depth(particle_t p);
int particle_get_lifetime(particle_t p);
int particle_get_life(particle_t p);
pextra* particle_get_extra(particle_t p);
#else
#define particle_is_alive(p) ((p)->life > 0)
#define particle_get_x(p) ((p)->x)
#define particle_get_y(p) ((p)->y)
#define particle_get_dx(p) ((p)->dx)
#define particle_get_dy(p) ((p)->dy)
#define particle_get_radius(p) ((p)->radius)
#define particle_get_depth(p) ((p)->depth)
#define particle_get_life(p) ((p)->life)
#define particle_get_lifetime(p) ((p)->lifetime)
#endif

#endif
