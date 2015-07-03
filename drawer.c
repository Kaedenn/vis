
#define _BSD_SOURCE /* for setenv */

#include "drawer.h"
#include "emitter.h"
#include "helper.h"
#include "kstring.h"
#include "particle_extra.h"
#include <errno.h>

#include <SDL_image.h>

static double calculate_blend(particle_t particle);

static const size_t FPS_COUNTER_LEN = 20;

struct fps {
    Uint32 framecount;
    Uint32 start;
    Uint32 framestart;
    double last_fps;
};

struct drawer {
    SDL_Window* window;
    SDL_Renderer* renderer;
    struct crect* rect_array;
    size_t rect_curr;
    size_t rect_count;
    float bgcolor[3];
    struct fps fps;
    BOOL tracing;
    emit_t emit_desc;
    BOOL verbose_trace;
    char* dump_file_fmt;
};

drawer_t drawer_new(void) {
    drawer_t drawer = DBMALLOC(sizeof(struct drawer));
#ifdef DEBUG
    setenv("SDL_DEBUG", "1", 1);
#endif
    /* initialize SDL */
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        eprintf("error in drawer_new SDL_Init: %s", SDL_GetError());
        return NULL;
    }
    drawer->window = SDL_CreateWindow("Vis",
                                      VIS_WINDOW_X, VIS_WINDOW_Y,
                                      VIS_WIDTH, VIS_HEIGHT, 0);
    if (drawer->window == NULL) {
        eprintf("error in SDL_CreateWindow: %s", SDL_GetError());
        return NULL;
    }
    drawer->renderer = SDL_CreateRenderer(drawer->window, -1,
                                          SDL_RENDERER_PRESENTVSYNC);
    /* initialize SDL_image */
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        eprintf("error in IMG_Init: %s", SDL_GetError());
        return NULL;
    }
    /* initialize the vertex storage */
    drawer->rect_curr = 0;
    drawer->rect_count = VIS_PLIST_INITIAL_SIZE * VIS_VTX_PER_PARTICLE;
    drawer->rect_array = DBMALLOC(drawer->rect_count * sizeof(struct crect));
    /* initialize default values */
    drawer_bgcolor(drawer, 0, 0, 0);
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
    DBFREE(drawer->rect_array);
    if (drawer->dump_file_fmt) {
        DBFREE(drawer->dump_file_fmt);
    }
    DBFREE(drawer->emit_desc);
    DBFREE(drawer);
    IMG_Quit();
    SDL_Quit();
}

void drawer_bgcolor(drawer_t drawer, float r, float g, float b) {
    drawer->bgcolor[0] = r;
    drawer->bgcolor[1] = g;
    drawer->bgcolor[2] = b;
}

int drawer_add_particle(drawer_t drawer, particle_t particle) {
    pextra_t pe = (pextra_t)particle->extra;
    if (drawer->rect_curr < drawer->rect_count) {
        struct crect* r = &drawer->rect_array[drawer->rect_curr];
        r->r.x = (int)particle->x - (int)particle->radius;
        r->r.y = (int)particle->y - (int)particle->radius;
        r->r.w = 2 * (int)particle->radius;
        r->r.h = 2 * (int)particle->radius;
        r->c.r = (Uint8)(pe->r*0xFF);
        r->c.g = (Uint8)(pe->g*0xFF);
        r->c.b = (Uint8)(pe->b*0xFF);
        r->c.a = (Uint8)(calculate_blend(particle)*0xFF);
        drawer->rect_curr += 1;
        return 0;
    } else {
        eprintf("can't add more than %d particles, did you call "
                "drawer_draw_to_screen?", drawer->rect_count);
        return 1;
    }
}

int drawer_draw_to_screen(drawer_t drawer) {
    kstr s = NULL;
    SDL_Surface* dumpsurf = NULL;
    if (drawer->dump_file_fmt) {
        s = kstring_newfromvf("%s_%04d.bmp", drawer->dump_file_fmt,
                              drawer->fps.framecount);
        Uint32 mask[4] = {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#else
            0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#endif
        };
        dumpsurf = SDL_CreateRGBSurface(0, VIS_WIDTH, VIS_HEIGHT, 32,
                                        mask[0], mask[1], mask[2], mask[3]);
        if (dumpsurf == NULL) {
            eprintf("Error creating surface for frame dump: %s",
                    SDL_GetError());
        } else {
            SDL_SetSurfaceBlendMode(dumpsurf, SDL_BLENDMODE_NONE);
            SDL_FillRect(dumpsurf, NULL, SDL_MapRGBA(dumpsurf->format,
                                                    0, 0, 0, 0xFF));
            SDL_SetSurfaceBlendMode(dumpsurf, SDL_BLENDMODE_BLEND);
        }
    }

    SDL_SetRenderDrawColor(drawer->renderer,
                           (Uint8)(drawer->bgcolor[0]*0xFF),
                           (Uint8)(drawer->bgcolor[1]*0xFF),
                           (Uint8)(drawer->bgcolor[2]*0xFF),
                           0xFF);
    SDL_RenderClear(drawer->renderer);
    SDL_SetRenderDrawBlendMode(drawer->renderer, SDL_BLENDMODE_BLEND);
    for (size_t i = 0; i < drawer->rect_curr; ++i) {
        SDL_SetRenderDrawColor(drawer->renderer,
                               drawer->rect_array[i].c.r,
                               drawer->rect_array[i].c.g,
                               drawer->rect_array[i].c.b,
                               drawer->rect_array[i].c.a);
        SDL_RenderFillRect(drawer->renderer, &drawer->rect_array[i].r);
        if (dumpsurf != NULL) {
            SDL_FillRect(dumpsurf, &drawer->rect_array[i].r,
                         SDL_MapRGBA(dumpsurf->format,
                                     drawer->rect_array[i].c.r,
                                     drawer->rect_array[i].c.g,
                                     drawer->rect_array[i].c.b,
                                     drawer->rect_array[i].c.a));
        }
    }
    SDL_RenderPresent(drawer->renderer);
    drawer->rect_curr = 0;

    if (drawer->dump_file_fmt) {
        if (dumpsurf != NULL) {
            SDL_SaveBMP(dumpsurf, kstring_content(s));
            SDL_FreeSurface(dumpsurf);
        }
        kstring_free(s);
    }
    
    Uint32 frameend = SDL_GetTicks();
    Uint32 framedelay = frameend - drawer->fps.framestart;
    if (framedelay < VIS_MSEC_PER_FRAME) {
        float fps = drawer_get_fps(drawer);
        Uint32 correction = (fps > VIS_FPS_LIMIT ? 1 : 0);
        SDL_Delay((Uint32)round(VIS_MSEC_PER_FRAME - framedelay + correction));
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
    SDL_RendererInfo ri;
    SDL_GetRendererInfo(drawer->renderer, &ri);
    if (ri.flags & SDL_RENDERER_TARGETTEXTURE) {
        drawer->dump_file_fmt = dupstr(path);
    }
}

void drawer_set_trace_verbose(drawer_t drawer, BOOL verbose) {
    drawer->verbose_trace = verbose;
}

void drawer_set_trace(drawer_t drawer, emit_t emit) {
    drawer->emit_desc = emit;
}

emit_t drawer_get_trace(drawer_t drawer) {
    return drawer->emit_desc;
}

void drawer_begin_trace(drawer_t drawer) {
    drawer->tracing = TRUE;
}

void drawer_trace(drawer_t drawer, float x, float y) {
    if (!drawer->tracing || !drawer->emit_desc) return;
    if (x < 0.0f || y < 0.0f || x > VIS_WIDTH*1.0f || y > VIS_HEIGHT*1.0f) {
        return;
    }

    drawer->emit_desc->x = (double)x;
    drawer->emit_desc->y = (double)y;
    emit_frame(drawer->emit_desc);
    if (drawer->verbose_trace) {
        char* line = genlua_emit(drawer->emit_desc, drawer->fps.framecount);
        printf("%s\n", line);
        DBFREE(line);
    }
}

void drawer_end_trace(drawer_t drawer) {
    drawer->tracing = FALSE;
}

double calculate_blend(particle_t particle) {
    pextra_t pe = (pextra_t)particle->extra;
    double alpha = pe->a;
    double life = particle_get_life(particle);
    double lifetime = particle_get_lifetime(particle);
    switch (pe->blender) {
        /* default blend is linear */
        case VIS_BLEND_LINEAR: {
            alpha *= linear_blend(life, lifetime);
        } break;
        case VIS_BLEND_QUADRATIC: {
            alpha *= quadratic_blend(life, lifetime);
        } break;
        case VIS_BLEND_NEGGAMMA: {
            alpha *= neggamma_blend(life, lifetime);
        } break;
        case VIS_BLEND_NONE: {
            alpha *= no_blend(life, lifetime);
        } break;
        case VIS_NBLENDS:
        default: { } break;
    }
    return alpha;
}

