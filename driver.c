
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
#include "draw.h"
#include "emitter.h"
#include "helper.h"
#include "particle_extra.h"
#include "plist.h"
#include "script.h"

plist_t particles = NULL;

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
    
    particles = plist_new(VIS_PLIST_INITIAL_SIZE);
    drawer = drawer_new();
    
    emitter_setup(particles);
    audio_init();
    
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
        command_setup(particles);
    }
    
    if (args.scriptfile) {
        s = script_new();
        emitter_schedule(script_run(s, args.scriptfile));
    }
    
    mainloop(drawer);

    if (args.interactive) {
        command_teardown();
        script_destroy(s);
    }
    
    return 0;
}

void mainloop(drawer_t drawer) {
    SDL_Event e;
    Uint32 lasttime = SDL_GetTicks();
    Uint32 ticktime;
    while (TRUE) {
        ticktime = SDL_GetTicks() - lasttime;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
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
            ticktime = SDL_GetTicks() - lasttime;
        }
        if (ticktime < 1000/VIS_FPS_LIMIT) {
            SDL_Delay(1000/VIS_FPS_LIMIT - ticktime);
        } else if (SDL_GetTicks() - lasttime >= 1000/VIS_FPS_LIMIT) {
            lasttime = SDL_GetTicks();
            display(drawer);
            timeout();
        }
    }
}

plist_action_t animate_particle(particle_t p, size_t idx, void* userdefined) {
    UNUSED_VARIABLE(idx);
    drawer_t drawer = (drawer_t)userdefined;

    /* TODO: add passive forces */
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
        case VIS_NO_BLEND: {
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
    plist_free(particles);
    audio_close();
}

