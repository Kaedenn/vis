
#ifndef VIS_DRAW_HEADER_INCLUDED_
#define VIS_DRAW_HEADER_INCLUDED_ 1

#include "defines.h"
#include "emit.h"
#include "particle.h"
#include <math.h>

/* Remove when drawer_draw_to_screen is complete */
#define DRAWER_DRAW_TO_SCREEN_IS_INCOMPLETE

#include <SDL.h>
#include <SDL_opengl.h>

typedef double(*blend_fn)(double, double);

typedef struct drawer* drawer_t;
struct crect {
    SDL_Rect r;
    SDL_Color c;
};

drawer_t drawer_new(void);
void drawer_free(drawer_t drawer);

void drawer_bgcolor(drawer_t drawer, GLfloat r, GLfloat g, GLfloat b);
int drawer_add_particle(drawer_t drawer, particle_t particle);
int drawer_draw_to_screen(drawer_t drawer);
float drawer_get_fps(drawer_t drawer);

void drawer_set_dumpfile_template(drawer_t drawer, const char* path);

void drawer_set_trace_verbose(drawer_t drawer, BOOL verbose);
void drawer_set_trace(drawer_t drawer, emit_t emit);
emit_t drawer_get_trace(drawer_t drawer);
void drawer_begin_trace(drawer_t drawer);
void drawer_trace(drawer_t drawer, float x, float y);
void drawer_end_trace(drawer_t drawer);

void vis_coords_to_screen(float x, float y, float* nx, float* ny);

/* constant through 0 and 1 */
static inline double no_blend(double current, UNUSED_PARAM(double max)) {
    return current;
}

/* linear through (0,0) and (max,1) */
static inline double linear_blend(double current, double max) {
    return current / max;
}

/* parabola through (0,0) and (max,1) */
static inline double quadratic_blend(double current, double max) {
    return -(current) * (current - 2 * max) / (max * max);
}

/* naive negative gamma through (0,0) */
static inline double neggamma_blend(double current, double max) {
    return 1 - exp(-(current * current) / (max * max));
}

/* following are in order of increasing execution time (or so I hope) */
static const blend_fn blend_fns[VIS_NBLENDS] = {
    no_blend,
    linear_blend,
    quadratic_blend,
    neggamma_blend
};

static const blend_t default_blender = VIS_BLEND_LINEAR;

#endif

