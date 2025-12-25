
#ifndef VIS_EMITTER_HEADER_INCLUDED_
#define VIS_EMITTER_HEADER_INCLUDED_ 1

#include "command.h"
#include "emit.h"
#include "flist.h"
#include "plist.h"

void emitter_setup(struct commands* cmds, plist_t plist, drawer_t drawer);
void emitter_free(void);

uint32_t emitter_get_frame_count(ftype_id ft);

void emitter_schedule(flist_t frames);
void emitter_schedule_frame(emit_desc* frame, unsigned int when);
void emitter_tick(void);

void emit_frame(emit_desc* frame);

#endif
