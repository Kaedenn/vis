
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
    BOOL should_exit;
    int exit_status;
};

void finalize(void);
void mainloop(struct global_ctx* ctx);

plist_action_t animate_particle(struct particle* p, size_t idx, void* userdefined);
void display(struct global_ctx* ctx);
void timeout(struct global_ctx* ctx);

void global_ctx_free(struct global_ctx* ctx);

int main(int argc, char* argv[]) {
    srand((unsigned)time(NULL));

    struct global_ctx global_ctx;
    ZEROINIT(&global_ctx);

    gc_init();
    
    global_ctx.args = argparse(argc, argv);
    if (!global_ctx.args) {
        exit(1);
    } else if (global_ctx.args->must_exit) {
        exit(global_ctx.args->exit_status);
    }
    gc_add((gc_func_t)free, global_ctx.args);

    global_ctx.drawer = drawer_new();
    if (!global_ctx.drawer) {
        exit(1);
    }
    gc_add((gc_func_t)drawer_free, global_ctx.drawer);
    drawer_config(global_ctx.drawer, global_ctx.args);
    
    global_ctx.particles = plist_new(VIS_PLIST_INITIAL_SIZE);
    gc_add((gc_func_t)plist_free, global_ctx.particles);
    
    global_ctx.script = script_new(SCRIPT_ALLOW_ALL);
    script_set_drawer(global_ctx.script, global_ctx.drawer);
    gc_add((gc_func_t)script_free, global_ctx.script);

    global_ctx.cmds = command_setup(global_ctx.drawer,
                                    global_ctx.particles,
                                    global_ctx.script,
                                    global_ctx.args->interactive);
    gc_add((gc_func_t)command_teardown, global_ctx.cmds);
    
    emitter_setup(global_ctx.cmds, global_ctx.particles);
    gc_add((gc_func_t)emitter_free, NULL);
    
    if (!audio_init()) {
        exit(1);
    }
    gc_add((gc_func_t)audio_free, NULL);
    
    global_ctx.args = argparse(argc, argv);
    gc_add((gc_func_t)free, global_ctx.args);
    
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

    if (global_ctx.args->scriptfile) {
        flist_t flist = NULL;
        flist = script_run(global_ctx.script, global_ctx.args->scriptfile);
        emitter_schedule(flist);
        gc_add((gc_func_t)flist_free, flist);
    }

    mainloop(&global_ctx);

    script_on_quit(global_ctx.script);

    if (global_ctx.exit_status < script_get_status(global_ctx.script)) {
        global_ctx.exit_status = script_get_status(global_ctx.script);
    }

    return global_ctx.exit_status;
}

void mainloop(struct global_ctx* ctx) {
    SDL_Event e;
    memset(&e, 0, sizeof(SDL_Event));
    while (!ctx->should_exit) {
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
                    script_keydown(ctx->script, SDL_GetKeyName(e.key.keysym.sym),
                                   e.key.keysym.mod & KMOD_SHIFT);
                    break;
                case SDL_KEYUP:
                    script_keyup(ctx->script, SDL_GetKeyName(e.key.keysym.sym),
                                 e.key.keysym.mod & KMOD_SHIFT);
                    break;
                break;
                case SDL_QUIT: return;
                default: { } break;
            }
        }
        struct script_debug dbg;
        script_get_debug(ctx->script, &dbg);

        script_set_debug(ctx->script, SCRIPT_DEBUG_FRAMES_EMITTED,
                         emitter_get_emit_frame_count());
        script_set_debug(ctx->script, SCRIPT_DEBUG_TIME_NOW,
                         SDL_GetTicks());
        script_set_debug(ctx->script, SCRIPT_DEBUG_NUM_MUTATES,
                         emitter_get_num_mutates());
        script_set_debug(ctx->script, SCRIPT_DEBUG_PARTICLES_EMITTED,
                         dbg.particles_emitted +
                            (uint32_t)plist_get_size(ctx->particles));
        int status;
        if ((status = script_get_status(ctx->script)) != 0) {
            ctx->should_exit = TRUE;
            ctx->exit_status = status;
        } else {
            display(ctx);
            timeout(ctx);
        }
    }
}
 
plist_action_t animate_particle(struct particle* p, size_t idx, void* userdefined) {
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
            if (command_should_exit(ctx->cmds)) {
                ctx->should_exit = TRUE;
                ctx->exit_status = command_get_error(ctx->cmds);
            }
            delayctr = 0;
        }
    }
    emitter_tick();
}

