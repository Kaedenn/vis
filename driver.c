
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "async.h"
#include "audio.h"
#include "clargs.h"
#include "command.h"
#include "defines.h"
#include "drawer.h"
#include "emitter.h"
#include "helper.h"
#include "particle_extra.h"
#include "plist.h"
#include "script.h"

plist_t particles = NULL;

typedef void (*gc_func_t)(void* cls);

struct gcitem {
    gc_func_t func;
    void* cls;
};

static struct gcitem* gcitems = NULL;
static size_t gcidx = 0;
static size_t ngcitems = 0;

static inline void gc_add(gc_func_t fn, void* cls) {
    gcitems[gcidx].func = fn;
    gcitems[gcidx].cls = cls;
    ++gcidx;
}

void gc(void);
void finalize(void);
void mainloop(drawer_t drawer);

plist_action_t animate_particle(particle_t p, size_t idx, void* userdefined);
void display(drawer_t drawer);
void timeout(void);

int main(int argc, char* argv[]) {
    SDL_Surface* screen = NULL;
    drawer_t drawer = NULL;
    script_t s = NULL;
    srand((unsigned)time(NULL));
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        eprintf("unable to initialize: %s", SDL_GetError());
        exit(1);
    }
    atexit(finalize);
    screen = SDL_SetVideoMode(VIS_WIDTH, VIS_HEIGHT, 32,
                              SDL_HWSURFACE | SDL_ASYNCBLIT |
                              SDL_OPENGLBLIT |
                              SDL_GL_DOUBLEBUFFER | SDL_OPENGL);
    if (screen == NULL) {
        eprintf("unable to create screen: %s", SDL_GetError());
        exit(1);
    }

    ngcitems = 10;
    gcitems = DBMALLOC(ngcitems * sizeof(struct gcitem));
    atexit(gc);
    
    particles = plist_new(VIS_PLIST_INITIAL_SIZE);
    drawer = drawer_new();

    gc_add((gc_func_t)drawer_free, drawer);
    gc_add((gc_func_t)plist_free, particles);
    
    emitter_setup(particles);
    gc_add((gc_func_t)emitter_free, NULL);
    audio_init();

    gc_add((gc_func_t)audio_free, NULL);
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glViewport(0, 0, VIS_WIDTH, VIS_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, VIS_WIDTH, VIS_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_TEXTURE_2D);
    glLoadIdentity();
    
    argparse(argc, argv);
    
    if (args.interactive) {
        emit_t emit = DBMALLOC(sizeof(struct emit_frame));
        emit->n = 100;
        emit->rad = 1;
        emit->life = 100;
        emit->ulife = 20;
        emit->ds = 0.2;
        emit->uds = 0.2;
        emit->theta = 0;
        emit->utheta = 2*M_PI;
        emit->ur = 0.2f;
        emit->ug = 1;
        emit->ub = 1;
        emit->limit = VIS_LIMIT_SPRINGBOX;
        emit->blender = VIS_BLEND_QUADRATIC;
        drawer_set_emit(drawer, emit);
        command_setup(particles);
    }
    
    if (args.scriptfile) {
        flist_t flist = NULL;
        s = script_new();
        script_set_drawer(s, drawer);
        flist = script_run(s, args.scriptfile);
        emitter_schedule(flist);
        gc_add((gc_func_t)script_destroy, s);
        gc_add((gc_func_t)flist_free, flist);
    }
    
    mainloop(drawer);

    if (args.interactive) {
        command_teardown();
    }

    return 0;
}

void gc(void) {
    size_t i;
    for (i = 0; i < ngcitems; ++i) {
        if (gcitems[i].func) {
            (gcitems[i].func)(gcitems[i].cls);
        }
    }
    DBFREE(gcitems);
}

void mainloop(drawer_t drawer) {
    SDL_Event e;
    memset(&e, 0, sizeof(SDL_Event));
    Uint32 lasttime = SDL_GetTicks();
    while (TRUE) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_MOUSEBUTTONDOWN:
                    drawer_begin_trace(drawer);
                    drawer_trace(drawer, (float)e.button.x, (float)e.button.y);
                    break;
                case SDL_MOUSEBUTTONUP:
                    drawer_end_trace(drawer);
                    break;
                case SDL_MOUSEMOTION:
                    drawer_trace(drawer, (float)e.motion.x, (float)e.motion.y);
                    break;
                case SDL_KEYDOWN: {
                    if (e.key.keysym.sym == SDLK_ESCAPE) {
                        return;
                    }
                    SDL_keysym* sym = &e.key.keysym;
                    DBPRINTF("Key scan=%x sym=%x mod=%x", (int)sym->scancode,
                             (int)sym->sym, (int)sym->mod);
                } break;
                case SDL_QUIT: {
                    return;
                } break;
                default: { } break;
            }
        }
        drawer_ensure_fps(drawer);
        /*
        DBPRINTF("now: %d, last: %d, delta: %d, mspf: %g",
                 SDL_GetTicks(), lasttime, (SDL_GetTicks() - lasttime),
                 (double)VIS_MSEC_PER_FRAME);
        */
        if (SDL_GetTicks() - lasttime < VIS_MSEC_PER_FRAME) {
            Uint32 correction = (drawer_get_fps(drawer) > 30.0) ? 1 : 0;
            SDL_Delay((Uint32)VIS_MSEC_PER_FRAME - (SDL_GetTicks() - lasttime) + correction);
        }
        lasttime = SDL_GetTicks();
        display(drawer);
        timeout();
    }
}
 
plist_action_t animate_particle(particle_t p, size_t idx, void* userdefined) {
    UNUSED_VARIABLE(idx);
    drawer_t drawer = (drawer_t)userdefined;

    pextra_t pe = (pextra_t)(p->extra);
    double life, lifetime;
    double alpha = 1;
    life = particle_get_life(p);
    lifetime = particle_get_lifetime(p);
    switch (pe->blender) {
        /* default blend is linear */
        case VIS_BLEND_LINEAR: {
            alpha = linear_blend(life, lifetime);
        } break;
        case VIS_BLEND_QUADRATIC: {
            alpha = quadratic_blend(life, lifetime);
        } break;
        case VIS_BLEND_NEGGAMMA: {
            alpha = neggamma_blend(life, lifetime);
        } break;
        case VIS_BLEND_NONE: {
            alpha = no_blend(life, lifetime);
        } break;
        case VIS_NBLENDS:
        default: { } break;
    }
    
    drawer_add_particle(drawer, p);
#ifndef notyet
    /* Remove when drawer_draw_to_screen is complete */
    glBegin(GL_POLYGON);
    glColor4d(pe->r, pe->g, pe->b, alpha);
    draw_diamond(p->x, p->y, p->radius);
    glEnd();
#endif
    
    particle_tick(p);
    if (!particle_is_alive(p)) {
        return ACTION_REMOVE;
    }
    return ACTION_NEXT;
}

void display(drawer_t drawer) {
#ifndef notyet
    /* Remove when drawer_draw_to_screen is complete */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
    plist_foreach(particles, animate_particle, drawer);
    drawer_draw_to_screen(drawer);
}

void timeout(void) {
    static int delayctr = 0;
    if (args.interactive) {
        if (++delayctr % VIS_CMD_DELAY_NSTEPS == 0) {
            command();
            delayctr = 0;
        }
    }
    emitter_tick();
}

void finalize(void) {
    SDL_Quit();
}

