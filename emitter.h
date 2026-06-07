
#ifndef VIS_EMITTER_HEADER_INCLUDED_
#define VIS_EMITTER_HEADER_INCLUDED_ 1

#include "script.h"
#include "clargs.h"
#include "emit.h"
#include "flist.h"
#include "plist.h"

void emitter_setup(script_t script, plist_t plist, drawer_t drawer, clargs_t args);
void emitter_free(void);

uint32_t emitter_get_frame_count(ftype_id ft);

/* Emitter takes ownership of the frame list; the caller must not free() it */
void emitter_schedule(flist_t frames);

void emitter_schedule_frame(emit_desc* frame, unsigned int when);
void emitter_tick(void);

void emit_frame(emit_desc* frame);

BOOL emitter_should_exit(void);

#endif
