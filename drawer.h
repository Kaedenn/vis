
#ifndef VIS_DRAW_HEADER_INCLUDED_
#define VIS_DRAW_HEADER_INCLUDED_ 1

#include "defines.h"
#include "emit.h"
#include "particle.h"
#include <math.h>

/* Remove when drawer_draw_to_screen is complete */
#define DRAWER_DRAW_TO_SCREEN_IS_INCOMPLETE

/* Pull in GLext functions */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#include <SDL_opengl.h>

/** Notes:
 *
 * glDrawElements with GL_TRIANGLES to draw a quad {v1, v2, v3, v4}:
 *      {v1, v2, v3,  v1, v3, v4}

//usage
glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
glEnableVertexAttribArray(0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(
   0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
   2,                  // size
   GL_FLOAT,           // type
   GL_FALSE,           // normalized?
   sizeof(vertex),                  // stride
   (void*)0            // array buffer offset
);

glVertexAttribPointer(
   1,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
   3,                  // size
   GL_FLOAT,           // type
   GL_FALSE,           // normalized?
   sizeof(vertex),                  // stride
   (void*)sizeof(vec2)            // array buffer offset
);

// Draw the triangle !
glDrawArrays(GL_TRIANGLES, 0, 24); // Starting from vertex 0; 24 vertices total

 */

typedef double(*blend_fn)(double, double);

typedef struct drawer* drawer_t;
typedef struct vertex {
    float x, y;
    float r, g, b;
} *vertex_t;

drawer_t drawer_new(void);
void drawer_free(drawer_t drawer);

int drawer_bgcolor(drawer_t drawer, GLfloat r, GLfloat g, GLfloat b);
int drawer_add_particle(drawer_t drawer, particle_t particle);
int drawer_draw_to_screen(drawer_t drawer);
float drawer_get_fps(drawer_t drawer);

void drawer_set_dumpfile_template(drawer_t drawer, const char* path);

void drawer_begin_trace(drawer_t drawer);
void drawer_set_emit(drawer_t, emit_t emit);
emit_t drawer_get_emit(drawer_t);
void drawer_trace(drawer_t drawer, float x, float y);
void drawer_end_trace(drawer_t drawer);

void vis_coords_to_screen(float x, float y, float* nx, float* ny);

static inline void set_background_color(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

static inline void draw_diamond(double x, double y, double radius) {
    glVertex3d(x         , y - radius, 1);
    glVertex3d(x + radius, y         , 1);
    glVertex3d(x         , y + radius, 1);
    glVertex3d(x - radius, y         , 1);
}

/* constant through 0 and 1 */
static inline double no_blend(double current, UNUSED_PARAM(double max)) {
    return current;
}

/* linear through (0,0) and (max,1) */
static inline double linear_blend(double current, double max) {
    return current / max;
}

/* parabola through (0,0) and (max,1) */
static inline double quadratic_blend(double current, double max) {
    return -(current) * (current - 2 * max) / (max * max);
}

/* naive negative gamma through (0,0) */
static inline double neggamma_blend(double current, double max) {
    return 1 - exp(-(current * current) / (max * max));
}

/* following are in order of increasing execution time (or so I hope) */
static const blend_fn blend_fns[VIS_NBLENDS] = {
    no_blend,
    linear_blend,
    quadratic_blend,
    neggamma_blend
};

static const blend_t default_blender = VIS_BLEND_LINEAR;

#endif

