
#include "drawer.h"
#include "helper.h"
#include "particle.h"
#include "particle_extra.h"
#include "emitter.h"
#include <SDL/SDL.h>

static const size_t FPS_COUNTER_LEN = 20;

struct fps {
    Uint32 framecount;
    Uint32 start;
    Uint32 framecount_brief;
    Uint32 start_brief;
    Uint32 framestart;
    double last_fps;
};

struct drawer {
    GLuint vbo;
    GLuint program_id;
    vertex_t vtx_array;
    size_t vtx_curr;
    size_t vtx_count;
    float bgcolor[3];
    struct fps fps;
    BOOL tracing;
    emit_t emit_desc;
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
    drawer_t drawer = DBMALLOC(sizeof(struct drawer));
    drawer->vbo = 0;
    drawer->program_id = 0;
    glGenBuffers(1, &drawer->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, drawer->vbo);
    drawer->vtx_curr = 0;
    drawer->vtx_count = VIS_PLIST_INITIAL_SIZE * VIS_VTX_PER_PARTICLE;
    drawer->vtx_array = DBMALLOC(drawer->vtx_count * sizeof(struct vertex));
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
    drawer->fps.start = SDL_GetTicks();
    drawer->fps.framestart = drawer->fps.start;
    return drawer;
}

void drawer_free(drawer_t drawer) {
    Uint32 runtime = SDL_GetTicks() - drawer->fps.start;
    double runtime_sec = (double)runtime / 1000.0;
    DBPRINTF("%d frames in %d ticks", drawer->fps.framecount, runtime);
    DBPRINTF("deduced fps: %g", (double)drawer->fps.framecount / runtime_sec);
    DBPRINTF("frame error: %g", runtime_sec * VIS_FPS_LIMIT - drawer->fps.framecount);
    DBFREE(drawer->vtx_array);
    /* FIXME: free vbo, program_id */
    DBFREE(drawer->emit_desc);
    DBFREE(drawer);
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
    
    drawer->fps.framecount += 1;
    drawer->fps.framecount_brief += 1;
    if (drawer->fps.framecount_brief == FPS_COUNTER_LEN) {
        Uint32 delta = SDL_GetTicks() - drawer->fps.start_brief;
        drawer->fps.last_fps = drawer->fps.framecount_brief * 1000.0 / delta;
        drawer->fps.framecount_brief = 0;
        drawer->fps.start_brief = SDL_GetTicks();
    }
    return 0;
}

void vis_coords_to_screen(float x, float y, float* nx, float* ny) {
    // (0, VIS_WIDTH) -> (-1, 1)
    // (0, VIS_HEIGHT) -> (-1, 1)
    *nx = 2 * x / VIS_WIDTH - 1;
    *ny = 2 * y / VIS_HEIGHT - 1;
}

void drawer_ensure_fps(drawer_t drawer) {
    Uint32 elapsed_msec = SDL_GetTicks() - drawer->fps.start;
    double elapsed_sec = (double)elapsed_msec / 1000.0;
    double frametime = 0; /* FIXME */
    /*DBPRINTF("Brief fps: %g", drawer->fps.last_fps);*/
}

float drawer_get_fps(drawer_t drawer) {
    return (float)drawer->fps.framecount / ((float)SDL_GetTicks() - (float)drawer->fps.start) * 1000.0f;
}

void drawer_begin_trace(drawer_t drawer) {
    drawer->tracing = TRUE;
}

void drawer_set_emit(drawer_t drawer, emit_t emit) {
    drawer->emit_desc = emit;
}

emit_t drawer_get_emit(drawer_t drawer) {
    return drawer->emit_desc;
}

void drawer_trace(drawer_t drawer, float x, float y) {
    if (!drawer->tracing || !drawer->emit_desc) return;
    if (x < 0.0f || y < 0.0f || x > VIS_WIDTH*1.0f || y > VIS_HEIGHT*1.0f) {
        return;
    }

    drawer->emit_desc->x = (double)x;
    drawer->emit_desc->y = (double)y;
    emit_frame(drawer->emit_desc);
}

void drawer_end_trace(drawer_t drawer) {
    drawer->tracing = FALSE;
}

