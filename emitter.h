
#ifndef VIS_EMITTER_HEADER_INCLUDED_
#define VIS_EMITTER_HEADER_INCLUDED_ 1

#include "plist.h"
#include "defines.h"
#include "flist.h"
#include "forces.h"
#include "limits.h"

#include <stdio.h>

void emitter_setup(plist_t particles);

void emitter_schedule(flist_t frames);
void emitter_schedule_frame(frame_t frame, unsigned int when);
void emitter_tick(void);

void emit_frame(frame_t frame);

#endif

