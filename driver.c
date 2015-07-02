
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL.h>

#include "async.h"
#include "audio.h"
#include "clargs.h"
#include "command.h"
#include "defines.h"
#include "drawer.h"
#include "emit.h"
#include "emitter.h"
#include "gc.h"
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
    drawer_t drawer = NULL;
    script_t s = NULL;
    srand((unsigned)time(NULL));

    gc_init();
    
    drawer = drawer_new();
    if (!drawer) {
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
    drawer_set_trace(drawer, emit);

    if (args.dumptrace) {
        drawer_set_trace_verbose(drawer, TRUE);
    }

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

void mainloop(drawer_t drawer) {
    SDL_Event e;
    memset(&e, 0, sizeof(SDL_Event));
    while (TRUE) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_MOUSEBUTTONDOWN:
                    drawer_begin_trace(drawer);
                case SDL_MOUSEMOTION:
                    drawer_trace(drawer, (float)e.button.x, (float)e.button.y);
                    break;
                case SDL_MOUSEBUTTONUP:
                    drawer_end_trace(drawer);
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
    drawer_add_particle(drawer, p);
    particle_tick(p);
    return particle_is_alive(p) ? ACTION_NEXT : ACTION_REMOVE;
}

void display(drawer_t drawer) {
    drawer_predraw(drawer);
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

