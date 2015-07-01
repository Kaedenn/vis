
#define _BSD_SOURCE /* for setenv */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_opengl.h>

#include "async.h"
#include "audio.h"
#include "clargs.h"
#include "command.h"
#include "defines.h"
#include "drawer.h"
#include "emit.h"
#include "emitter.h"
#include "helper.h"
#include "particle_extra.h"
#include "plist.h"
#include "script.h"

plist_t particles = NULL;

#ifndef GC_MIN_SIZE
#define GC_MIN_SIZE 10
#endif

typedef void (*gc_func_t)(void* cls);

struct gcitem {
    gc_func_t func;
    void* cls;
};

static struct gcitem* gcitems = NULL;
static size_t gcidx = 0;
static size_t ngcitems = 0;

static inline void gc_add(gc_func_t fn, void* cls) {
    if (gcidx == ngcitems) {
        gcitems = realloc(gcitems, 2*ngcitems);
    }
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

#ifdef DEBUG
    setenv("SDL_DEBUG", "1", 1);
#endif

    ngcitems = GC_MIN_SIZE;
    gcitems = DBMALLOC(ngcitems * sizeof(struct gcitem));
    atexit(gc);
    
    drawer = drawer_new();
    if (!drawer) {
        const char* sdlerr = SDL_GetError();
        const char* oserr = strerror(errno);
        const char* sdldesc = sdlerr ? "SDL Error: " : "";
        const char* osdesc = oserr? "OS Error: " : "";
        eprintf("unable to initialize drawer: %s%s, %s%s",
                sdldesc, sdlerr, osdesc, oserr);
        exit(1);
    } else {
        gc_add((gc_func_t)drawer_free, drawer);
    }

    particles = plist_new(VIS_PLIST_INITIAL_SIZE);
    gc_add((gc_func_t)plist_free, particles);
    
    emitter_setup(particles);
    gc_add((gc_func_t)emitter_free, NULL);
    
    audio_init();
    gc_add((gc_func_t)audio_free, NULL);
    
    argparse(argc, argv);
    
    emit_t emit = emit_new();
    emit->n = 100;
    emit->rad = 1;
    emit_set_life(emit, 100, 200);
    emit_set_ds(emit, 0.2, 0.2);
    emit_set_angle(emit, 0, 2*M_PI);
    emit_set_color(emit, 0, 0, 0, 0.2f, 1, 1);
    emit->limit = VIS_LIMIT_SPRINGBOX;
    emit->blender = VIS_BLEND_QUADRATIC;
    drawer_set_emit(drawer, emit);

    if (args.dumpfile) {
        drawer_set_dumpfile_template(drawer, args.dumpfile);
    }

    if (args.interactive) {
        command_setup(particles);
    }
    
    if (args.scriptfile) {
        flist_t flist = NULL;
        s = script_new(SCRIPT_ALLOW_ALL);
        script_set_drawer(s, drawer);
        flist = script_run(s, args.scriptfile);
        emitter_schedule(flist);
        gc_add((gc_func_t)script_free, s);
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
        display(drawer);
        timeout();
    }
}
 
plist_action_t animate_particle(particle_t p, size_t idx, void* userdefined) {
    UNUSED_VARIABLE(idx);
    drawer_t drawer = (drawer_t)userdefined;

    pextra_t pe = (pextra_t)(p->extra);
    double life, lifetime;
    double alpha = pe->a;
    life = particle_get_life(p);
    lifetime = particle_get_lifetime(p);
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
    
    drawer_add_particle(drawer, p);
#ifdef DRAWER_DRAW_TO_SCREEN_IS_INCOMPLETE
    /* Remove when drawer_draw_to_screen is complete */
    glBegin(GL_POLYGON);
    glColor4d(pe->r, pe->g, pe->b, alpha);
    draw_diamond(p->x, p->y, p->radius);
    glEnd();
#endif
    
    particle_tick(p);
    return particle_is_alive(p) ? ACTION_NEXT : ACTION_REMOVE;
}

void display(drawer_t drawer) {
#ifdef DRAWER_DRAW_TO_SCREEN_IS_INCOMPLETE
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

