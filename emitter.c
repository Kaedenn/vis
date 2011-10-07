
#include "emitter.h"

#include "audio.h"
#include "command.h"
#include "helper.h"
#include "particle.h"
#include "particle_extra.h"

#include "forces.h"
#include "limits.h"
#include "draw.h"

#include "random.h"

#include <string.h>

static plist_t particles = NULL;
static flist_t fl = NULL;

void emitter_setup(plist_t plist) {
  particles = plist;
  fl = flist_new();
}

void emitter_schedule(flist_t frames) {
  flist_clear(fl);
  flist_free(fl);
  fl = frames;
}

void emitter_tick(void) {
  flist_node_t fn = flist_tick(fl);
  while (fn != NULL) {
    switch (fn->type) {
      case VIS_FTYPE_EMIT:
        emit_frame(fn->data.frame);
        break;
      case VIS_FTYPE_CMD:
        docommand(fn->data.cmd);
        break;
      default:
        break;
    }
    fn = fn->next;
  }
}

void emit_frame(frame_t frame) {
  int i;
  for (i = 0; i < frame->n; ++i) {
    particle_t p = NULL;
    pextra_t pe = NULL;
    float r, g, b;
    r = randfloat(frame->r - frame->ur, frame->r + frame->ur);
    g = randfloat(frame->g - frame->ug, frame->g + frame->ug);
    b = randfloat(frame->b - frame->ub, frame->b + frame->ub);
    pe = new_particle_extra(r, g, b, frame->blender);
    p = particle_new_full(frame->x, frame->y, frame->ux, frame->uy,
                          frame->rad, frame->urad, frame->ds, frame->uds,
                          frame->theta, frame->utheta,
                          frame->life, frame->ulife,
                          frame->force, frame->limit, pe);
    plist_add(particles, p);
  }
}

