
#include "drawer.h"
#include "helper.h"
#include "kstring.h"
#include "particle.h"
#include "particle_extra.h"
#include "emitter.h"
#include <SDL.h>

static const size_t FPS_COUNTER_LEN = 20;

struct fps {
    Uint32 framecount;
    Uint32 start;
    Uint32 framestart;
    double last_fps;
};

struct drawer {
    SDL_Surface* screen;
    GLuint vbo;
    GLuint program_id;
    vertex_t vtx_array;
    size_t vtx_curr;
    size_t vtx_count;
    float bgcolor[3];
    struct fps fps;
    BOOL tracing;
    emit_t emit_desc;
    char* dump_file_fmt;
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
    /* initialize SDL */
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        return NULL;
    }
    drawer->screen = SDL_SetVideoMode(VIS_WIDTH, VIS_HEIGHT, 32,
            SDL_HWSURFACE | SDL_ASYNCBLIT | SDL_OPENGLBLIT |
            SDL_GL_DOUBLEBUFFER | SDL_OPENGL);
    if (drawer->screen == NULL) {
        return NULL;
    }
    /* initialize OpenGL basics */
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glViewport(0, 0, VIS_WIDTH, VIS_HEIGHT);
    glMatrixMode(GL_PROJECTION); /* init projection matrix */
    glLoadIdentity();
    glOrtho(0, VIS_WIDTH, VIS_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW); /* init model view matrix */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_TEXTURE_2D);
    glLoadIdentity();
    /* initialize OpenGL shader engine */
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
    /* TODO: load shaders, compile program */

    /* initialize fps analysis */
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
    if (drawer->dump_file_fmt) {
        DBFREE(drawer->dump_file_fmt);
    }
    DBFREE(drawer->emit_desc);
    DBFREE(drawer);
    SDL_Quit();
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
    Uint32 frameend, framedelay;
#ifdef notyet
    /* Not until shaders are working */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, (GLint)drawer->vtx_curr);
#else
    SDL_GL_SwapBuffers();
    drawer->vtx_curr = 0;
#endif
    
    frameend = SDL_GetTicks();
    framedelay = frameend - drawer->fps.framestart;
    if (framedelay < VIS_MSEC_PER_FRAME) {
        float fps = drawer_get_fps(drawer);
        Uint32 correction = (fps > VIS_FPS_LIMIT ? 1 : 0);
        SDL_Delay((Uint32)round(VIS_MSEC_PER_FRAME - framedelay + correction));
    }
    if (drawer->dump_file_fmt) {
        kstr s = kstring_newfromvf("%s_%03d.png", drawer->dump_file_fmt,
                                   drawer->fps.framecount);
        /*
        SDL_Surface* tmp = SDL_PNGFormatAlpha(drawer->screen);
        SDL_SavePNG(tmp, kstring_content(s));
        SDL_FreeSurface(tmp);
        */
        kstring_free(s);
    }
    drawer->fps.framecount += 1;
    drawer->fps.framestart = SDL_GetTicks();
    return 0;
}

void vis_coords_to_screen(float x, float y, float* nx, float* ny) {
    *nx = 2 * x / VIS_WIDTH - 1;
    *ny = 2 * y / VIS_HEIGHT - 1;
}

float drawer_get_fps(drawer_t drawer) {
    return (float)drawer->fps.framecount / (float)(SDL_GetTicks() - drawer->fps.start) * 1000.0f;
}

void drawer_set_dumpfile_template(drawer_t drawer, const char* path) {
    drawer->dump_file_fmt = dupstr(path);
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

