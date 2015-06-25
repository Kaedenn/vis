
#ifndef VIS_TYPES_HEADER_INCLUDED_
#define VIS_TYPES_HEADER_INCLUDED_ 1

typedef unsigned int fnum_t;
typedef unsigned int frame_type_t;

/* force functions */
typedef enum {
    /* default force is no force */
    VIS_DEFAULT_FORCE = 0,
    VIS_FORCE_FRICTION,
    VIS_FORCE_GRAVITY,
    VIS_NFORCES
} force_t;

/* position-limiting functions */
typedef enum {
    /* default limit is no limit */
    VIS_DEFAULT_LIMIT = 0,
    VIS_LIMIT_BOX,
    VIS_LIMIT_SPRINGBOX,
    VIS_NLIMITS
} limit_t;

/* mutation functions */
typedef enum {
    VIS_MUTATE_PUSH = 0,
    VIS_MUTATE_PUSH_DX,
    VIS_MUTATE_PUSH_DY,
    VIS_MUTATE_SLOW,
    VIS_MUTATE_SHRINK,
    VIS_MUTATE_GROW,
    VIS_NMUTATES
} mutate_t;

/* alpha-channel blending functions */
typedef enum {
    VIS_BLEND_NONE = 0,
    /* default blend is linear */
    VIS_DEFAULT_BLEND = 1,
    VIS_BLEND_LINEAR = 1,
    VIS_BLEND_QUADRATIC,
    VIS_BLEND_NEGGAMMA,
    VIS_NBLENDS
} blend_t;


/* constants for frame types */
typedef enum {
    VIS_FTYPE_EMIT = 0,
    VIS_FTYPE_CMD,
    VIS_FTYPE_BGCOLOR,
    VIS_FTYPE_MUTATE,
    VIS_FTYPE_SCRIPTCB,
    VIS_FTYPE_FRAMESEEK,

    VIS_MAX_FTYPE
} ftype_t;

typedef struct emit_frame {
    int n;              /* number of particles to emit */
    double x, y;        /* where to emit from */
    double ux, uy;      /* uncertainty in x and y */
    double rad;         /* radius */
    double urad;        /* uncertainty in rad */
    int life;           /* particle lifetime */
    int ulife;          /* uncertainty in life (haha) */
    double ds;          /* initial velocity */
    double uds;         /* uncertainty in ds */
    double theta;       /* initial angle */
    double utheta;      /* uncertainty in theta */
    float r, g, b;      /* color */
    float ur, ug, ub;   /* uncertainty in r, g and b */
    force_t force;      /* force function */
    limit_t limit;      /* limit function */
    blend_t blender;   /* blending function */
} *emit_t;

void dbprintf_emit_t(emit_t e);

#endif
