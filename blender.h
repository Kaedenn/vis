
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

static const blend_id default_blender = VIS_BLEND_LINEAR;

#endif
