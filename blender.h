
#ifndef VIS_BLENDER_HEADER_INCLUDED_
#define VIS_BLENDER_HEADER_INCLUDED_ 1

#include "helper.h"
#include "types.h"
#include <math.h>

typedef double(*blend_fn)(double, double);

/* constant through 0 and 1 */
static inline double no_blend(double current, UNUSED_PARAM(double max)) {
    return current;
}

/* linear through (0,0) and (max,1) */
static inline double linear_blend(double current, double max) {
    return current / max;
}

/* parabola through (0,0) and (max/2,1) and (max,0) */
static inline double parabolic_blend(double current, double max) {
    /* FIXME: math */
    return -4 * (current/max) * (current/max - 1);
}

/* parabola through (0,0) and (max,1) */
static inline double quadratic_blend(double current, double max) {
    return -(current) * (current - 2 * max) / (max * max);
}

/* sine curve from 0 to 1 */
static inline double sine_blend(double current, double max) {
    return sin(M_PI * current/max);
}

/* naive negative gamma through (0,0) */
static inline double neggamma_blend(double current, double max) {
    return 1 - exp(-(current * current) / (max * max));
}

/* ease from 0 to 1 and back to 0 */
static inline double easing_blend(double current, double max) {
    /* f(x)=e^(-a(x-1/2)^p), a = 500, p = 6*/
    return exp(-500*pow(current/max - 1.0/2.0, 6));
}

/* following are in order of increasing execution time (or so I hope) */
static const blend_fn blend_fns[VIS_NBLENDS] = {
    no_blend,
    linear_blend,
    parabolic_blend,
    quadratic_blend,
    sine_blend,
    neggamma_blend,
    easing_blend
};

static const blend_id default_blender = VIS_BLEND_LINEAR;

#endif
