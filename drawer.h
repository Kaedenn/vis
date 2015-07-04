
#ifndef VIS_DRAW_HEADER_INCLUDED_
#define VIS_DRAW_HEADER_INCLUDED_ 1

#include "blender.h"
#include "defines.h"
#include "emit.h"
#include "particle.h"

typedef struct drawer* drawer_t;

drawer_t drawer_new(void);
void drawer_free(drawer_t drawer);

void drawer_bgcolor(drawer_t drawer, float r, float g, float b);
int drawer_add_particle(drawer_t drawer, particle_t particle);
int drawer_draw_to_screen(drawer_t drawer);
float drawer_get_fps(drawer_t drawer);

void drawer_set_dumpfile_template(drawer_t drawer, const char* path);

void drawer_set_trace_verbose(drawer_t drawer, BOOL verbose);
void drawer_set_trace(drawer_t drawer, emit_t emit);
emit_t drawer_get_trace(drawer_t drawer);
void drawer_begin_trace(drawer_t drawer);
void drawer_trace(drawer_t drawer, float x, float y);
void drawer_end_trace(drawer_t drawer);

#endif

