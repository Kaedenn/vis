
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

struct global_ctx {
    plist_t particles;
    drawer_t drawer;
    struct clargs* args;
    struct commands* cmds;
    script_t script;
};

void finalize(void);
void mainloop(struct global_ctx* ctx);

plist_action_t animate_particle(particle_t p, size_t idx, void* userdefined);
void display(struct global_ctx* ctx);
void timeout(struct global_ctx* ctx);

void global_ctx_free(struct global_ctx* ctx);

int main(int argc, char* argv[]) {
    srand((unsigned)time(NULL));

    struct global_ctx global_ctx;
    ZEROINIT(&global_ctx);

    gc_init();
    
    global_ctx.args = argparse(argc, argv);
    gc_add((gc_func_t)free, global_ctx.args);

    global_ctx.drawer = drawer_new();
    if (!global_ctx.drawer) {
        exit(1);
    }
    gc_add((gc_func_t)drawer_free, global_ctx.drawer);
    drawer_config(global_ctx.drawer, global_ctx.args);
    
    global_ctx.particles = plist_new(VIS_PLIST_INITIAL_SIZE);
    gc_add((gc_func_t)plist_free, global_ctx.particles);
    
    emitter_setup(global_ctx.cmds, global_ctx.particles);
    gc_add((gc_func_t)emitter_free, NULL);
    
    audio_init();
    gc_add((gc_func_t)audio_free, NULL);
    
    global_ctx.args = argparse(argc, argv);
    gc_add((gc_func_t)free, global_ctx.args);
    
    global_ctx.script = script_new(SCRIPT_ALLOW_ALL);
    script_set_drawer(global_ctx.script, global_ctx.drawer);
    gc_add((gc_func_t)script_free, global_ctx.script);

    emit_t emit = emit_new();
    emit->n = 100;
    emit->rad = 1;
    emit_set_life(emit, 100, 200);
    emit_set_ds(emit, 0.2, 0.2);
    emit_set_angle(emit, 0, 2*M_PI);
    emit_set_color(emit, 0, 0, 0, 0.2f, 1, 1);
    emit->limit = VIS_LIMIT_SPRINGBOX;
    emit->blender = VIS_BLEND_QUADRATIC;
    drawer_set_trace(global_ctx.drawer, emit);

    if (global_ctx.args->interactive) {
        global_ctx.cmds = command_setup(global_ctx.drawer,
                                        global_ctx.particles,
                                        global_ctx.script);
        gc_add((gc_func_t)command_teardown, global_ctx.cmds);
    }
    
    if (global_ctx.args->scriptfile) {
        flist_t flist = NULL;
        flist = script_run(global_ctx.script, global_ctx.args->scriptfile);
        emitter_schedule(flist);
        gc_add((gc_func_t)flist_free, flist);
    }

    /* MUST be the last thing added to the gc */
    gc_add((gc_func_t)script_quit, global_ctx.script);
    
    mainloop(&global_ctx);

    return 0;
}

void mainloop(struct global_ctx* ctx) {
    SDL_Event e;
    memset(&e, 0, sizeof(SDL_Event));
    while (TRUE) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_MOUSEBUTTONDOWN:
                    drawer_begin_trace(ctx->drawer);
                    drawer_trace(ctx->drawer, (float)e.button.x,
                                 (float)e.button.y);
                    script_mousedown(ctx->script, e.button.x, e.button.y);
                    break;
                case SDL_MOUSEMOTION:
                    drawer_trace(ctx->drawer, (float)e.motion.x,
                                 (float)e.motion.y);
                    script_mousemove(ctx->script, e.motion.x, e.motion.y);
                    break;
                case SDL_MOUSEBUTTONUP:
                    drawer_end_trace(ctx->drawer);
                    script_mouseup(ctx->script, e.button.x, e.button.y);
                    break;
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_ESCAPE) {
                        return;
                    }
                    script_keydown(ctx->script, SDL_GetKeyName(e.key.keysym.sym));
                    break;
                case SDL_KEYUP:
                    script_keyup(ctx->script, SDL_GetKeyName(e.key.keysym.sym));
                    break;
                break;
                case SDL_QUIT: return;
                default: { } break;
            }
        }
        display(ctx);
        timeout(ctx);
    }
}
 
plist_action_t animate_particle(particle_t p, size_t idx, void* userdefined) {
    UNUSED_VARIABLE(idx);
    struct global_ctx* ctx = (struct global_ctx*)userdefined;
    drawer_add_particle(ctx->drawer, p);
    particle_tick(p);
    return particle_is_alive(p) ? ACTION_NEXT : ACTION_REMOVE;
}

void display(struct global_ctx* ctx) {
    plist_foreach(ctx->particles, animate_particle, ctx);
    drawer_draw_to_screen(ctx->drawer);
}

void timeout(struct global_ctx* ctx) {
    static int delayctr = 0;
    if (ctx->args->interactive) {
        if (++delayctr % VIS_CMD_DELAY_NSTEPS == 0) {
            command(ctx->cmds);
            delayctr = 0;
        }
    }
    emitter_tick();
}

