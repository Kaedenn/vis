
#include "draw.h"
#include "helper.h"
#include "particle.h"
#include "particle_extra.h"
#include <SDL/SDL.h>

struct drawer {
    GLuint vbo;
    GLuint program_id;
    vertex_t vtx_array;
    size_t vtx_curr;
    size_t vtx_count;
    float bgcolor[3];
};

enum shader_t {
    SHADER_VERTEX,
    SHADER_FRAGMENT
};

/*
static GLuint load_shader(const char* path, enum shader_t shader_type) {
    UNUSED_VARIABLE(path);
    UNUSED_VARIABLE(shader_type);
    // http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/#Shaders
    return 0;
}
*/

drawer_t drawer_new(void) {
    drawer_t drawer = chmalloc(sizeof(struct drawer));
    drawer->vbo = 0;
    drawer->program_id = 0;
    glGenBuffers(1, &drawer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, drawer->vbo);
    drawer->vtx_curr = 0;
    drawer->vtx_count = VIS_PLIST_INITIAL_SIZE * VIS_VTX_PER_PARTICLE;
    drawer->vtx_array = chmalloc(drawer->vtx_count * sizeof(struct vertex));
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(drawer->vtx_count * sizeof(struct vertex)),
                 drawer->vtx_array, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
                          (void*)0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex),
                          (void*)(sizeof(float)*2));
    drawer_bgcolor(drawer, 0, 0, 0);
    /* FIXME: load shaders, compile program */
    return drawer;
}

void drawer_free(drawer_t drawer) {
    free(drawer->vtx_array);
    /* FIXME: free vbo, program_id */
    free(drawer);
}

int drawer_bgcolor(drawer_t drawer, GLfloat r, GLfloat g, GLfloat b) {
    drawer->bgcolor[0] = r;
    drawer->bgcolor[1] = g;
    drawer->bgcolor[2] = b;
    return 0;
}

int drawer_add_particle(drawer_t drawer, particle_t particle) {
    pextra_t pe = (pextra_t)particle->extra;
    if (drawer->vtx_curr < drawer->vtx_count) {
        vertex_t vtx = &drawer->vtx_array[drawer->vtx_curr];
        vis_coords_to_screen((float)particle->x, (float)particle->y,
                             &vtx->x, &vtx->y);
        vtx->r = pe->r;
        vtx->g = pe->g;
        vtx->b = pe->b;
        drawer->vtx_curr += 1;
        return 0;
    } else {
        eprintf("can't add more than %d particles, did you call "
                "drawer_draw_to_screen?", drawer->vtx_count);
        return 1;
    }
}

int drawer_draw_to_screen(drawer_t drawer) {
#ifdef notyet
    /* Not until shaders are working */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, (GLint)drawer->vtx_curr);
#endif
    SDL_GL_SwapBuffers();
    drawer->vtx_curr = 0;
    return 0;
}

void vis_coords_to_screen(float x, float y, float* nx, float* ny) {
    // (0, VIS_WIDTH) -> (-1, 1)
    // (0, VIS_HEIGHT) -> (-1, 1)
    *nx = 2 * x / VIS_WIDTH - 1;
    *ny = 2 * y / VIS_HEIGHT - 1;
}
