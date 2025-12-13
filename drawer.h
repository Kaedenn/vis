
#ifndef VIS_DRAW_HEADER_INCLUDED_
#define VIS_DRAW_HEADER_INCLUDED_ 1

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "clargs.h"
#include "emit.h"
#include "particle.h"

typedef struct drawer* drawer_t;

drawer_t drawer_new(const clargs* args);
void drawer_free(drawer_t drawer);
GLFWwindow* drawer_get_window(drawer_t drawer);

void drawer_config(drawer_t drawer, const clargs* args);
void drawer_bgcolor(drawer_t drawer, float r, float g, float b);
int drawer_add_particle(drawer_t drawer, particle* p);
int drawer_draw_to_screen(drawer_t drawer);
void drawer_preserve_screen(drawer_t drawer);
float drawer_get_fps(drawer_t drawer);

void drawer_set_dumpfile_template(drawer_t drawer, const char* path);

void drawer_set_trace(drawer_t drawer, emit_desc* emit);
void drawer_begin_trace(drawer_t drawer);
void drawer_trace(drawer_t drawer, float x, float y);
void drawer_end_trace(drawer_t drawer);

#endif
