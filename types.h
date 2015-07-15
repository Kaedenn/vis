
#ifndef VIS_TYPES_HEADER_INCLUDED_
#define VIS_TYPES_HEADER_INCLUDED_ 1

#include <stdint.h>

typedef unsigned int fnum;

union particle_tag {
    int64_t l;
    uint64_t ul;
    struct { int32_t h; int32_t l; } i;
    struct { uint32_t h; uint32_t l; } ui;
};

/* force functions */
typedef enum {
    /* default force is no force */
    VIS_DEFAULT_FORCE = 0,
    VIS_FORCE_FRICTION,
    VIS_FORCE_GRAVITY,
    VIS_NFORCES,
} force_id;

/* position-limiting functions */
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
    VIS_MUTATE_TAG_SET,
    VIS_MUTATE_TAG_INC,
    VIS_MUTATE_TAG_DEC,
    VIS_MUTATE_TAG_ADD,
    VIS_MUTATE_TAG_SUB,
    VIS_MUTATE_TAG_MUL,
    VIS_MUTATE_TAG_DIV,
    VIS_MUTATE_PUSH_IF,
    VIS_MUTATE_PUSH_DX_IF,
    VIS_MUTATE_PUSH_DY_IF,
    VIS_MUTATE_SLOW_IF,
    VIS_MUTATE_SHRINK_IF,
    VIS_MUTATE_GROW_IF,
    VIS_MUTATE_AGE_IF,
    VIS_MUTATE_OPACITY_IF,
    VIS_NMUTATES,
} mutate_id;

/* mutation conditions */
typedef enum {
    VIS_MUTATE_IF_TRUE,
    VIS_MUTATE_IF_EQ,
    VIS_MUTATE_IF_NE,
    VIS_MUTATE_IF_LT,
    VIS_MUTATE_IF_LE,
    VIS_MUTATE_IF_GT,
    VIS_MUTATE_IF_GE,
    VIS_MUTATE_IF_EVEN,
    VIS_MUTATE_IF_ODD,
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
    VIS_FTYPE_EMIT = 0,
    VIS_FTYPE_EXIT,
    VIS_FTYPE_PLAY,
    VIS_FTYPE_CMD,
    VIS_FTYPE_BGCOLOR,
    VIS_FTYPE_MUTATE,
    VIS_FTYPE_SCRIPTCB,
    VIS_FTYPE_FRAMESEEK,

    VIS_MAX_FTYPE,
} ftype_id;

#endif
