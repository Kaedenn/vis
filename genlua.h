
#ifndef VIS_GENLUA_HEADER_INCLUDED_
#define VIS_GENLUA_HEADER_INCLUDED_ 1

#include "types.h"
#include "emit.h"

char* genlua_emit(emit_desc* emit, fnum when);
const char* genlua_force(force_id force);
const char* genlua_limit(limit_id limit);
const char* genlua_blender(blend_id blender);
const char* genlua_mutate(mutate_id fnid);
const char* genlua_mutate_cond(mutate_cond_id id);

#endif
