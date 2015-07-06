
#ifndef VIS_EMITTER_HEADER_INCLUDED_
#define VIS_EMITTER_HEADER_INCLUDED_ 1

#include "plist.h"
#include "defines.h"
#include "command.h"
#include "emit.h"
#include "flist.h"
#include "forces.h"
#include "plimits.h"

void emitter_setup(struct commands* cmds, plist_t particles);
void emitter_free(void* arg);

void emitter_schedule(flist_t frames);
void emitter_schedule_frame(emit_t frame, unsigned int when);
void emitter_tick(void);

void emit_frame(emit_t frame);

#endif

