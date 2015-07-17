
#define _BSD_SOURCE /* for setenv */

#include "drawer.h"
#include "emitter.h"
#include "genlua.h"
#include "helper.h"
#include "kstring.h"
#include "particle_extra.h"
#include <errno.h>

#include <SDL_image.h>

#ifndef MAX
#define MAX(a,b) ((a)<(b) ? (a) : (b))
#endif

static double calculate_blend(particle* p);
static int render_to_file(SDL_Renderer* renderer, const char* path);

void drawer_ensure_fps_linear(drawer_t drawer);
void drawer_ensure_fps_absolute(drawer_t drawer);

/* combine a rect and a color */
struct crect {
    SDL_Rect r;
    SDL_Color c;
};

/* used, obviously, for fps tracking and limiting */
struct fps {
    void (*limiter)(drawer_t drawer);
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
    SDL_Texture* pixel;
    Uint32 frame_skip;
    struct fps fps;
    BOOL tracing;
    emit_desc emit_desc;
    BOOL verbose_trace;
    double scale_factor;
    char* dump_file_fmt;
};

drawer_t drawer_new(void) {
    drawer_t drawer = DBMALLOC(sizeof(struct drawer));
#if DEBUG > DEBUG_NONE
    setenv("SDL_DEBUG", "1", 1);
#endif
    /* initialize SDL */
    if (SDL_Init(SDL_INIT_TIMER |
                 SDL_INIT_AUDIO |
                 SDL_INIT_VIDEO |
                 SDL_INIT_EVENTS) < 0) {
        EPRINTF("error in drawer_new SDL_Init: %s", SDL_GetError());
        return NULL;
    }
    drawer->window = SDL_CreateWindow("Vis",
                                      VIS_WINDOW_X, VIS_WINDOW_Y,
                                      VIS_WIDTH, VIS_HEIGHT, 0);
    if (drawer->window == NULL) {
        EPRINTF("error in SDL_CreateWindow: %s", SDL_GetError());
        return NULL;
    }
    drawer->renderer = SDL_CreateRenderer(drawer->window, -1,
                                          SDL_RENDERER_PRESENTVSYNC);
    /* initialize SDL_image */
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        EPRINTF("error in IMG_Init: %s", SDL_GetError());
        return NULL;
    }
    /* initialize the vertex storage */
    drawer->rect_curr = 0;
    drawer->rect_count = VIS_PLIST_INITIAL_SIZE * VIS_VTX_PER_PARTICLE;
    drawer->rect_array = DBMALLOC(drawer->rect_count * sizeof(struct crect));
    /* initialize default values */
    drawer_bgcolor(drawer, 0, 0, 0);
    drawer->scale_factor = 1.0;
    /* load the generic pixel */
    SDL_Surface* pixel = IMG_Load("pixel_3x3.png");
    if (!pixel) {
        EPRINTF("Failed to load pixel image: %s", SDL_GetError());
    } else if (!(drawer->pixel = SDL_CreateTextureFromSurface(drawer->renderer,
                                                              pixel))) {
        EPRINTF("Failed to create pixel texture: %s", SDL_GetError());
    } else {
        SDL_SetTextureBlendMode(drawer->pixel, SDL_BLENDMODE_BLEND);
    }
    if (pixel) {
        SDL_FreeSurface(pixel);
    }
    /* initialize fps analysis */
    drawer->fps.start = SDL_GetTicks();
    drawer->fps.framestart = drawer->fps.start;
    drawer->fps.limiter = drawer_ensure_fps_linear;
    return drawer;
}

void drawer_free(drawer_t drawer) {
    double runtime = 1.0 * SDL_GetTicks() - drawer->fps.start;
    double runtime_sec = runtime / 1000.0;
    double fc_want = runtime_sec * VIS_FPS_LIMIT;
    double fc_have = drawer->fps.framecount;
    DBPRINTF("%s", "fps analysis:");
    DBPRINTF("S=%g, S_ms=%g, F=%g, F/S=%g", runtime_sec, runtime, fc_have,
             drawer->fps.framecount / runtime_sec);
    DBPRINTF("frame error:   (S*FPS-F) %g", fc_want - fc_have);
    DBPRINTF("error ratio: 1-(S*FPS/F) %g", 1 - fc_want / fc_have);
    DBFREE(drawer->rect_array);
    if (drawer->dump_file_fmt) {
        DBFREE(drawer->dump_file_fmt);
    }
    if (drawer->pixel) {
        SDL_DestroyTexture(drawer->pixel);
    }
    SDL_DestroyRenderer(drawer->renderer);
    SDL_DestroyWindow(drawer->window);
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

int drawer_add_particle(drawer_t drawer, particle* p) {
    pextra* pe = (pextra*)p->extra;
    if (drawer->rect_curr < drawer->rect_count) {
        struct crect* r = &drawer->rect_array[drawer->rect_curr];
        int diameter = MAX((int)(drawer->scale_factor * p->radius), 3);
        int radius = diameter / 2;

        if (drawer->pixel) {
            r->r.x = (int)(p->x - radius * drawer->scale_factor);
            r->r.y = (int)(p->y - radius * drawer->scale_factor);
            r->r.w = r->r.h = (int)(diameter * drawer->scale_factor);
        } else {
            r->r.x = (int)p->x - radius;
            r->r.y = (int)p->y - radius;
            r->r.w = r->r.h = diameter;
        }

        r->c.r = (Uint8)(pe->r*0xFF);
        r->c.g = (Uint8)(pe->g*0xFF);
        r->c.b = (Uint8)(pe->b*0xFF);
        r->c.a = (Uint8)(sqrt(calculate_blend(p))*0xFF);
        drawer->rect_curr += 1;
        return 0;
    } else {
        EPRINTF("can't add more than %d particles, did you call "
                "drawer_draw_to_screen?", drawer->rect_count);
        return 1;
    }
}

int drawer_draw_to_screen(drawer_t drawer) {
    kstr s = NULL;
    SDL_Texture* dump_tex = NULL;
    SDL_Renderer* renderer = drawer->renderer;
    if (drawer->dump_file_fmt) {
        if (drawer->fps.framecount >= drawer->frame_skip) {
            s = kstring_newfromvf("%s_%04d.png", drawer->dump_file_fmt,
                                  drawer->fps.framecount - drawer->frame_skip);
        }
        dump_tex = SDL_CreateTexture(drawer->renderer,
                                     SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_TARGET,
                                     VIS_WIDTH, VIS_HEIGHT);
        if (dump_tex == NULL) {
            EPRINTF("Error creating dumping texture: %s",
                    SDL_GetError());
        } else {
            SDL_SetRenderTarget(drawer->renderer, dump_tex);
        }
    }
    SDL_SetRenderDrawBlendMode(drawer->renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(drawer->renderer,
                           (Uint8)(drawer->bgcolor[0]*0xFF),
                           (Uint8)(drawer->bgcolor[1]*0xFF),
                           (Uint8)(drawer->bgcolor[2]*0xFF),
                           0xFF);
    SDL_RenderClear(drawer->renderer);
    SDL_Rect final_pos;
    for (size_t i = 0; i < drawer->rect_curr; ++i) {
        SDL_SetRenderDrawColor(drawer->renderer,
                               drawer->rect_array[i].c.r,
                               drawer->rect_array[i].c.g,
                               drawer->rect_array[i].c.b,
                               drawer->rect_array[i].c.a);
        final_pos = drawer->rect_array[i].r;
        if (drawer->dump_file_fmt) {
            final_pos.y = VIS_HEIGHT - final_pos.y;
        }
        if (drawer->pixel) {
            SDL_SetTextureColorMod(drawer->pixel,
                                   drawer->rect_array[i].c.r,
                                   drawer->rect_array[i].c.g,
                                   drawer->rect_array[i].c.b);
            SDL_SetTextureAlphaMod(drawer->pixel, drawer->rect_array[i].c.a);
            SDL_RenderCopy(drawer->renderer, drawer->pixel, NULL, &final_pos);
        } else {
            SDL_RenderFillRect(drawer->renderer, &final_pos);
        }
    }
    SDL_RenderPresent(drawer->renderer);
    drawer->rect_curr = 0;

    if (drawer->dump_file_fmt) {
        if (s) {
            render_to_file(drawer->renderer, kstring_content(s));
            kstring_free(s);
        }
        if (dump_tex != NULL) {
            SDL_DestroyTexture(dump_tex);
        }
    } else if (drawer->fps.framecount >= drawer->frame_skip) {
        drawer->fps.limiter(drawer);
    }
    drawer->fps.framecount += 1;
    drawer->fps.framestart = SDL_GetTicks();
    return 0;
}

void drawer_ensure_fps_linear(drawer_t drawer) {
    Uint32 frameend = SDL_GetTicks();
    Uint32 framedelay = frameend - drawer->fps.framestart;
    if (framedelay < VIS_MSEC_PER_FRAME) {
        float fps = drawer_get_fps(drawer);
        Uint32 correction = (fps > VIS_FPS_LIMIT ? 1 : 0);
        SDL_Delay((Uint32)round(VIS_MSEC_PER_FRAME - framedelay + correction));
    }
}

void drawer_ensure_fps_absolute(drawer_t drawer) {
    /* For an absolute fps limiter:
     *  T_ms(frame) = T_ms(0) + VIS_MSEC_PER_FRAME * frame
     *  T_ms(0)     = (drawer->fps.start)
     * Therefore:
     *  dT_ms(frame) = T_ms(frame) - (drawer->fps.start)
     *
     * SDL_Delay((Uint32)round(dT_ms(frame)))
     */
    Uint32 frametime = (Uint32)round(drawer->fps.start +
                                     VIS_MSEC_PER_FRAME *
                                     drawer->fps.framecount - 1);
    Uint32 now = SDL_GetTicks();
    if (now < frametime) {
        SDL_Delay(frametime - now);
    }
}

float drawer_get_fps(drawer_t drawer) {
    return (float)(drawer->fps.framecount - 1) /
           (float)(SDL_GetTicks() - drawer->fps.start) * 1000.0f;
}

void drawer_config(drawer_t drawer, struct clargs* clargs) {
    drawer->frame_skip = (Uint32)clargs->frameskip;
    drawer->verbose_trace = clargs->dumptrace ? TRUE : FALSE;
    drawer->scale_factor = clargs->enlarge_particles ? 2.0 : 1.0;
    if (clargs->dumpfile) {
        SDL_RendererInfo ri;
        SDL_GetRendererInfo(drawer->renderer, &ri);
        if (ri.flags & SDL_RENDERER_TARGETTEXTURE) {
            drawer->dump_file_fmt = dupstr(clargs->dumpfile);
        } else {
            EPRINTF("SDL texture targets disabled, not dumping to file \"%s\"",
                    clargs->dumpfile);
        }
    }
    if (clargs->absolute_fps) {
        drawer->fps.limiter = drawer_ensure_fps_absolute;
    }
    DBPRINTF("%s", "Configured drawer:");
    if (drawer->frame_skip > 0) {
        DBPRINTF("\tdrawer->frame_skip = %d", drawer->frame_skip);
    }
    if (drawer->verbose_trace) {
        DBPRINTF("\t%s", "drawer->verbose_trace = TRUE");
    }
    if (clargs->enlarge_particles) {
        DBPRINTF("\tdrawer->scale_factor = %g", drawer->scale_factor);
    }
    if (drawer->dump_file_fmt != NULL) {
        DBPRINTF("\tdrawer->dumpfile = \"%s\"", drawer->dump_file_fmt);
    }
    DBPRINTF("drawer is using the %s fps limiter", 
             clargs->absolute_fps ? "absolute" : "linear");
}

void drawer_scale_particles(drawer_t drawer, double factor) {
    drawer->scale_factor = factor;
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

void drawer_set_trace(drawer_t drawer, emit_desc emit) {
    drawer->emit_desc = emit;
}

emit_desc drawer_get_trace(drawer_t drawer) {
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

double calculate_blend(particle* p) {
    pextra* pe = (pextra*)p->extra;
    double alpha = pe->a;
    double life = particle_get_life(p);
    double lifetime = particle_get_lifetime(p);
    if (pe->blender >= VIS_BLEND_NONE && pe->blender < VIS_NBLENDS) {
        alpha *= blend_fns[pe->blender](life, lifetime);
    }
    return alpha;
}

static int render_to_file(SDL_Renderer* renderer, const char* path) {
    void* pixels = DBMALLOC(4 * VIS_WIDTH * VIS_HEIGHT);
    int pitch = 4 * VIS_WIDTH;
    if (SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA8888, pixels,
                             pitch)) {
        DBFREE(pixels);
        EPRINTF("Failed to read pixels: %s", SDL_GetError());
        return 0;
    }

    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom(pixels, VIS_WIDTH, VIS_HEIGHT,
                                                 32, pitch,
                                                 0xFF000000, 0x00FF0000,
                                                 0x0000FF00, 0x000000FF);
    if (surf == NULL) {
        DBFREE(pixels);
        EPRINTF("Failed to alloc surface: %s", SDL_GetError());
        return 0;
    }

    IMG_SavePNG(surf, path);

    SDL_FreeSurface(surf);
    DBFREE(pixels);
    return 1;
}

