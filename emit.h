#ifndef VIS_EMIT_HEADER_INCLUDED_
#define VIS_EMIT_HEADER_INCLUDED_ 1

#include "helper.h"
#include "types.h"

/** main emit frame structure
 *
 * All fields are public.
 *
 * Uncertainty affects how each individual particle exists. For each particle,
 * their true x value is a random value between (x-ux, x+ux), and so on for
 * all values with an uncertainty.
 *
 * The functions below are provided for convenience; their use is not
 * required. Direct access to struct emit members is encouraged.
 *
 * Setting force, limit, or blender fields to improper values may cause
 * abnormal program termination.
 */

typedef struct emit {
    int n;              /* number of particles to emit */
    double x, y;        /* where to emit from */
    double ux, uy;      /* uncertainty in x and y */
    double rad;         /* radius */
    double urad;        /* uncertainty in rad */
    int life;           /* particle lifetime */
    int ulife;          /* uncertainty in life (ha) */
    double ds;          /* initial velocity */
    double uds;         /* uncertainty in ds */
    double theta;       /* initial angle */
    double utheta;      /* uncertainty in theta */
    float r, g, b;      /* color */
    float ur, ug, ub;   /* uncertainty in r, g and b */
    force_id force;      /* force function */
    limit_id limit;      /* limit function */
    blend_id blender;   /* blending function */
} *emit_desc;

emit_desc make_emit_frame(int n,
        double x, double y, double ux, double uy,
        double rad, double urad, double ds, double uds,
        double theta, double utheta, int life, int ulife,
        float r, float g, float b, float ur, float ub, float ug,
        force_id force, limit_id limit, blend_id blend); /* sorry */

emit_desc emit_new(void);
void emit_free(emit_desc emit);

void emit_set_n(emit_desc e, int n);
void emit_set_pos(emit_desc e, double x, double y, double dx, double dy);
void emit_set_rad(emit_desc e, double rad, double urad);
void emit_set_life(emit_desc e, int life, int ulife);
void emit_set_ds(emit_desc e, double ds, double uds);
void emit_set_angle(emit_desc e, double theta, double utheta);
void emit_set_color(emit_desc e, float r, float g, float b,
                    float ur, float ug, float ub);
void emit_set_force(emit_desc e, force_id force);
void emit_set_limit(emit_desc e, limit_id limit);
void emit_set_blender(emit_desc e, blend_id blender);

void dbprintf_emit_desc(emit_desc e);

#endif

