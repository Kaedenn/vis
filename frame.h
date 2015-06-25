
#ifndef VIS_FRAME_HEADER_INCLUDED_
#define VIS_FRAME_HEADER_INCLUDED_ 1

#include "defines.h"
#include "drawer.h"
#include "forces.h"
#include "limits.h"
#include "mutator.h"

/*
emit_t make_kick_frame(int n);
emit_t make_strum_frame(int n);
emit_t make_snare_frame(int n);
emit_t make_rain_frame(int n);
*/
emit_t make_emit_frame(int n,
        double x, double y, double ux, double uy,
        double rad, double urad, double ds, double uds,
        double theta, double utheta, int life, int ulife,
        float r, float g, float b, float ur, float ub, float ug,
        force_t force, limit_t limit, blend_t blend); /* sorry */

void free_frame(emit_t frame);

#endif

