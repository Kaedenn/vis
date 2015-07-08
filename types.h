
#ifndef VIS_TYPES_HEADER_INCLUDED_
#define VIS_TYPES_HEADER_INCLUDED_ 1

typedef unsigned int fnum_t;

/* force functions */
typedef enum {
    /* default force is no force */
    VIS_DEFAULT_FORCE = 0,
    VIS_FORCE_FRICTION,
    VIS_FORCE_GRAVITY,
    VIS_NFORCES
} force_id;

/* position-limiting functions */
typedef enum {
    /* default limit is no limit */
    VIS_DEFAULT_LIMIT = 0,
    VIS_LIMIT_BOX,
    VIS_LIMIT_SPRINGBOX,
    VIS_NLIMITS
} limit_id;

/* mutation functions */
typedef enum {
    VIS_MUTATE_PUSH = 0,
    VIS_MUTATE_PUSH_DX,
    VIS_MUTATE_PUSH_DY,
    VIS_MUTATE_SLOW,
    VIS_MUTATE_SHRINK,
    VIS_MUTATE_GROW,
    VIS_MUTATE_AGE,
    VIS_MUTATE_OPACITY,
    VIS_NMUTATES
} mutate_id;

/* alpha-channel blending functions */
typedef enum {
    VIS_BLEND_NONE = 0,
    /* default blend is linear */
    VIS_DEFAULT_BLEND = 1,
    VIS_BLEND_LINEAR = 1,
    VIS_BLEND_PARABOLIC,
    VIS_BLEND_QUADRATIC,
    VIS_BLEND_SINE,
    VIS_BLEND_NEGGAMMA,
    VIS_BLEND_EASING,
    VIS_NBLENDS
} blend_id;

/* constants for frame types */
typedef enum {
    VIS_FTYPE_EMIT = 0,
    VIS_FTYPE_EXIT,
    VIS_FTYPE_CMD,
    VIS_FTYPE_BGCOLOR,
    VIS_FTYPE_MUTATE,
    VIS_FTYPE_SCRIPTCB,
    VIS_FTYPE_FRAMESEEK,

    VIS_MAX_FTYPE
} ftype_id;

#endif
