
#ifndef VIS_TYPES_HEADER_INCLUDED_
#define VIS_TYPES_HEADER_INCLUDED_ 1

#include <stdint.h>

/* Frame number */
typedef unsigned int fnum_t;

/* Milliseconds */
typedef unsigned int msec_t;

/* Eight bytes of arbitrary data */
union particle_tag {
    int64_t l;
    uint64_t ul;
    struct { int32_t h; int32_t l; } i;
    struct { uint32_t h; uint32_t l; } ui;
    int16_t s[sizeof(uint64_t)/sizeof(int16_t)];
    uint16_t us[sizeof(uint64_t)/sizeof(uint16_t)];
    int8_t b[sizeof(uint64_t)/sizeof(int8_t)];
    uint8_t ub[sizeof(uint64_t)/sizeof(uint8_t)];
};

/* force functions (applied to dx, dy) */
typedef enum {
    /* default force is no force */
    VIS_DEFAULT_FORCE = 0,
    VIS_FORCE_FRICTION,
    VIS_FORCE_GRAVITY,
    VIS_NFORCES,
} force_id;

/* position-limiting functions (applied to x, y) */
typedef enum {
    /* default limit is no limit */
    VIS_DEFAULT_LIMIT = 0,
    VIS_LIMIT_BOX,
    VIS_LIMIT_SPRINGBOX,
    VIS_NLIMITS,
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
    VIS_MUTATE_SET_DX,
    VIS_MUTATE_SET_DY,
    VIS_MUTATE_SET_RADIUS,

    VIS_MUTATE_TAG_SET,
    VIS_MUTATE_TAG_INC,
    VIS_MUTATE_TAG_DEC,
    VIS_MUTATE_TAG_ADD,
    VIS_MUTATE_TAG_SUB,
    VIS_MUTATE_TAG_MUL,
    VIS_MUTATE_TAG_DIV,
    VIS_MUTATE_TAG_SET_IF,

    VIS_MUTATE_PUSH_IF,
    VIS_MUTATE_PUSH_DX_IF,
    VIS_MUTATE_PUSH_DY_IF,
    VIS_MUTATE_SLOW_IF,
    VIS_MUTATE_SHRINK_IF,
    VIS_MUTATE_GROW_IF,
    VIS_MUTATE_AGE_IF,
    VIS_MUTATE_OPACITY_IF,
    VIS_MUTATE_SET_DX_IF,
    VIS_MUTATE_SET_DY_IF,
    VIS_MUTATE_SET_RADIUS_IF,

    VIS_NMUTATES,
} mutate_id;

/* mutation conditions */
typedef enum {
    /* mutate based on value of particle tag */
    VIS_MUTATE_IF_TRUE,     /* always mutate */
    VIS_MUTATE_IF_EQ,
    VIS_MUTATE_IF_NE,
    VIS_MUTATE_IF_LT,
    VIS_MUTATE_IF_LE,
    VIS_MUTATE_IF_GT,
    VIS_MUTATE_IF_GE,
    VIS_MUTATE_IF_EVEN,
    VIS_MUTATE_IF_ODD,
    /* mutate based on particle location */
    VIS_MUTATE_IF_ABOVE,
    VIS_MUTATE_IF_BELOW,
    VIS_MUTATE_IF_LEFT,
    VIS_MUTATE_IF_RIGHT,

    VIS_MUTATE_IF_NEAR,
    VIS_MUTATE_IF_FAR,

    VIS_MUTATE_NCONDS,
} mutate_cond_id;

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
    VIS_NBLENDS,
} blend_id;

/* constants for frame types */
typedef enum {
    VIS_FTYPE_EMIT,
    VIS_FTYPE_EXIT,
    VIS_FTYPE_PLAY,
    VIS_FTYPE_PAUSE,
    VIS_FTYPE_VOLUME,
    VIS_FTYPE_AUDIOSEEK,
    VIS_FTYPE_CMD,
    VIS_FTYPE_BGCOLOR,
    VIS_FTYPE_MUTATE,
    VIS_FTYPE_SCRIPTCB,
    VIS_FTYPE_FRAMESEEK,
    VIS_FTYPE_DELAY,
    VIS_FTYPE_AUDIOSYNC,
    VIS_MAX_FTYPE,
} ftype_id;

#endif
