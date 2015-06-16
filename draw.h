
#ifndef VIS_DRAW_HEADER_INCLUDED_
#define VIS_DRAW_HEADER_INCLUDED_ 1

#include "defines.h"
#include <GL/gl.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592653579
#endif

typedef double(*blend_fn)(double, double);

static inline void set_background_color(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

static inline void draw_diamond(double x, double y, double radius) {
    glVertex3d(x         , y - radius, 1);
    glVertex3d(x + radius, y         , 1);
    glVertex3d(x         , y + radius, 1);
    glVertex3d(x - radius, y         , 1);
}

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

