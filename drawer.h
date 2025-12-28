
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
int drawer_get_configured_fps(drawer_t drawer);
float drawer_get_fps(drawer_t drawer);

/* Configure the output file prefix.
 * Frames are written to the file
 *    <path>_%04d.png
 * where the %04d is the current frame number. */
void drawer_set_dumpfile_template(drawer_t drawer, const char* path);

/* Configure the emit generated when clicking on the screen. Drawer
 * object takes control of the emit pointer and is responsible for
 * freeing it. Do not free the emit pointer after calling
 * drawer_set_trace. */
void drawer_set_trace(drawer_t drawer, emit_desc* emit);
emit_desc* drawer_get_trace(drawer_t drawer);
void drawer_begin_trace(drawer_t drawer);
void drawer_trace(drawer_t drawer, float x, float y);
void drawer_end_trace(drawer_t drawer);

void drawer_trace_scroll(drawer_t drawer, float xoffset, float yoffset);

#endif
