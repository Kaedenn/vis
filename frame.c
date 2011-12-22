
#include "frame.h"
#include "helper.h"

#include "forces.h"
#include "limits.h"
#include "draw.h"

/* for M_PI */
#define __USE_BSD

#include <math.h>

/*
frame_t make_kick_frame(int n) {
  return make_emit_frame(n,
    VIS_WIDTH / 2, VIS_HEIGHT / 2, 0.0, 0.0,
    1.5, 1.0, VIS_FPS_LIMIT * 3 / 2, VIS_FPS_LIMIT / 2,
    3.1 / 2, 2.9 / 2, M_PI, M_PI,
    1.5f, 0.5f, 0.35f, 0.35f, 0.2f, 0.2f,
    VIS_FRICTION, VIS_DEFAULT_LIMIT, VIS_BLEND_LINEAR);
}

frame_t make_strum_frame(int n) {
  return make_emit_frame(n,
    VIS_WIDTH / 2, VIS_HEIGHT / 2, 0.0, 0.0,
    1.5, 1.0, VIS_FPS_LIMIT * 3 / 2, VIS_FPS_LIMIT / 2,
    3.1 / 2, 2.9 / 2, M_PI, M_PI,
    0.2f, 0.2f, 0.35f, 0.35f, 0.5f, 0.5f,
    VIS_FRICTION, VIS_DEFAULT_LIMIT, VIS_BLEND_LINEAR);
}

frame_t make_snare_frame(int n) {
  return make_emit_frame(n,
    VIS_WIDTH / 2, VIS_HEIGHT / 2, 0.0, 0.0,
    1.5, 1.0, VIS_FPS_LIMIT * 3 / 2, VIS_FPS_LIMIT / 2,
    1.05, 1.0, M_PI, M_PI,
    0.2f, 0.2f, 0.5f, 0.5f, 0.2f, 0.2f,
    VIS_GRAVITY, VIS_SPRINGBOX, VIS_BLEND_LINEAR);
}

frame_t make_rain_frame(int n) {
  return make_emit_frame(n,
    0, 0, VIS_WIDTH, 0,
    1.0, 0.5, VIS_FPS_LIMIT * 5, VIS_FPS_LIMIT / 5,
    0.5, 0.5, M_PI * 3 / 2, 0,
    0.0f, 0.2f, 0.7f, 0.0f, 0.1f, 0.3f,
    VIS_GRAVITY, VIS_DEFAULT_LIMIT, VIS_NO_BLEND);
}
*/

/* n x y ux uy rad urad ds uds theta utheta life ulife r g b ur ug ub force limit blender */
frame_t make_emit_frame(int n,
    double x, double y, double ux, double uy,
    double rad, double urad, double ds, double uds,
    double theta, double utheta, int life, int ulife,
    float r, float g, float b, float ur, float ub, float ug,
    force_t force, limit_t limit, blend_t blend) /* sorry */ {
  frame_t frame = chmalloc(sizeof(struct emit_frame));
  frame->n = n;
  frame->x = x;
  frame->y = y;
  frame->ux = ux;
  frame->uy = uy;
  frame->rad = rad;
  frame->urad = urad;
  frame->life = life;
  frame->ulife = ulife;
  frame->ds = ds;
  frame->uds = uds;
  frame->theta = theta;
  frame->utheta = utheta;
  frame->r = r;
  frame->b = b;
  frame->g = g;
  frame->ur = ur;
  frame->ub = ub;
  frame->ug = ug;
  frame->force = force;
  frame->limit = limit;
  frame->blender = blend;
  return frame;
}

void free_frame(frame_t frame) {
  free(frame);
}

