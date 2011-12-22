
#ifndef VIS_FRAME_HEADER_INCLUDED_
#define VIS_FRAME_HEADER_INCLUDED_ 1

#include "defines.h"
#include "draw.h"
#include "forces.h"
#include "limits.h"

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
} *frame_t;

/*
frame_t make_kick_frame(int n);
frame_t make_strum_frame(int n);
frame_t make_snare_frame(int n);
frame_t make_rain_frame(int n);
*/
frame_t make_emit_frame(int n,
    double x, double y, double ux, double uy,
    double rad, double urad, double ds, double uds,
    double theta, double utheta, int life, int ulife,
    float r, float g, float b, float ur, float ub, float ug,
    force_t force, limit_t limit, blend_t blend); /* sorry */

void free_frame(frame_t frame);

#endif

