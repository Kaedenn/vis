
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
#include "mutator.h"
#include "pextra.h"
#include "plist.h"
#include "script.h"

/* dependencies
 *
 *  drawer -> particles
 *  script -> drawer
 *  command -> drawer, particles, script
 *  emitter -> command, particles
 */

struct global_ctx {
    plist_t particles;
    drawer_t drawer;
    clargs* args;
    struct commands* cmds;
    script_t script;
    BOOL paused;
    BOOL should_exit;
    int exit_status;
};

static void doevents(struct global_ctx* ctx);
static void mainloop(struct global_ctx* ctx);
static void animate(struct global_ctx* ctx);
static void display(struct global_ctx* ctx);
static void advance(struct global_ctx* ctx);
static void onkeydown(int key, struct global_ctx* ctx);
static plist_action_id animate_particle(struct particle* p, void* userdata);

int main(int argc, char* argv[]) {
    srand((unsigned)time(NULL));

#ifdef VIS_SELF_TEST
    VIS_ASSERT(sizeof(MUTATE_MAP)/sizeof(mutate_fn) == VIS_NMUTATES + 1);
#endif

    struct global_ctx g;
    ZEROINIT(&g);

    gc_init();
    
    g.args = argparse(argc, argv);
    if (!g.args) {
        exit(1);
    } else if (g.args->must_exit) {
        int status = g.args->exit_status;
        clargs_free(g.args);
        exit(status);
    }
    gc_add((gc_func)clargs_free, g.args);

    g.drawer = drawer_new();
    if (!g.drawer) {
        exit(1);
    }
    gc_add((gc_func)drawer_free, g.drawer);
    drawer_config(g.drawer, g.args);
    
    g.particles = plist_new(VIS_PLIST_INITIAL_SIZE);
    gc_add((gc_func)plist_free, g.particles);
    
    script_cfg_mask mask = SCRIPT_ALLOW_ALL;
    if (g.args->stay_after_script) {
        mask |= SCRIPT_NO_EXIT;
    }
    g.script = script_new(mask);
    if (klist_length(g.args->scriptargs) > 0) {
        script_set_args(g.script, g.args->scriptargs);
    } else {
        klist_free(g.args->scriptargs);
        g.args->scriptargs = NULL;
    }
    script_set_drawer(g.script, g.drawer);
    gc_add((gc_func)script_free, g.script);

    g.cmds = command_setup(g.drawer, g.particles, g.script,
                           g.args->interactive);
    gc_add((gc_func)command_teardown, g.cmds);
    
    emitter_setup(g.cmds, g.particles, g.drawer);
    gc_add((gc_func)emitter_free, NULL);
    
    if (!audio_init()) {
        exit(1);
    }
    if (g.args->quiet_audio) {
        audio_mute();
    }
    gc_add((gc_func)audio_free, NULL);
    
    emit_desc* emit = emit_new();
    emit->n = 100;
    emit->rad = 1;
    emit_set_life(emit, 100, 200);
    emit_set_ds(emit, 0.2, 0.2);
    emit_set_angle(emit, 0, 2*M_PI);
    emit_set_color(emit, 0, 0, 0, 0.2f, 1, 1);
    emit->limit = VIS_LIMIT_SPRINGBOX;
    emit->blender = VIS_BLEND_QUADRATIC;
    drawer_set_trace(g.drawer, emit);

    if (g.args->scriptfile) {
        emitter_schedule(script_run(g.script, g.args->scriptfile));
    }

    mainloop(&g);

    script_on_quit(g.script);

    if (g.args->scriptstring) {
        script_run_string(g.script, g.args->scriptstring);
    }

    if (g.exit_status < script_get_status(g.script)) {
        g.exit_status = script_get_status(g.script);
    }

    return g.exit_status;
}

void onkeydown(int sym, struct global_ctx* ctx) {
    switch (sym) {
        case SDLK_ESCAPE:
            ctx->should_exit = TRUE;
            break;
        case SDLK_SPACE:
            if (ctx->paused) {
                audio_play();
            } else {
                audio_pause();
            }
            ctx->paused = !ctx->paused;
            break;
        default:
            break;
    }
}

void doevents(struct global_ctx* ctx) {
    SDL_Event e;
    SDL_zero(e);
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_MOUSEBUTTONDOWN:
                drawer_begin_trace(ctx->drawer);
                drawer_trace(ctx->drawer, (float)e.button.x,
                             (float)e.button.y);
                script_mousedown(ctx->script, e.button.x, e.button.y,
                                 e.button.button);
                break;
            case SDL_MOUSEMOTION:
                drawer_trace(ctx->drawer, (float)e.motion.x,
                             (float)e.motion.y);
                script_mousemove(ctx->script, e.motion.x, e.motion.y);
                break;
            case SDL_MOUSEBUTTONUP:
                drawer_end_trace(ctx->drawer);
                script_mouseup(ctx->script, e.button.x, e.button.y,
                               e.button.button);
                break;
            case SDL_KEYDOWN:
                onkeydown(e.key.keysym.sym, ctx);
                script_keydown(ctx->script, SDL_GetKeyName(e.key.keysym.sym),
                               e.key.keysym.mod & KMOD_SHIFT);
                break;
            case SDL_KEYUP:
                script_keyup(ctx->script, SDL_GetKeyName(e.key.keysym.sym),
                             e.key.keysym.mod & KMOD_SHIFT);
                break;
            case SDL_QUIT:
                ctx->should_exit = TRUE;
                break;
            default: { } break;
        }
    }
}

void mainloop(struct global_ctx* ctx) {
    SDL_Event e;
    while (!ctx->should_exit) {
        doevents(ctx);

        struct script_debug dbg;
        script_get_debug(ctx->script, &dbg);

        script_set_debug(ctx->script, SCRIPT_DEBUG_FRAMES_EMITTED,
                         emitter_get_frame_count(VIS_FTYPE_EMIT));
        script_set_debug(ctx->script, SCRIPT_DEBUG_TIME_NOW,
                         SDL_GetTicks());
        script_set_debug(ctx->script, SCRIPT_DEBUG_NUM_MUTATES,
                         emitter_get_frame_count(VIS_FTYPE_MUTATE));
        script_set_debug(ctx->script, SCRIPT_DEBUG_PARTICLES_EMITTED,
                         dbg.particles_emitted +
                            plist_get_size(ctx->particles));
#if DEBUG >= DEBUG_DEBUG
        script_set_debug(ctx->script, SCRIPT_DEBUG_PARTICLES_MUTATED,
                        mutate_debug_get_particles_mutated());
        script_set_debug(ctx->script, SCRIPT_DEBUG_PARTICLE_TAGS_MODIFIED,
                        mutate_debug_get_particle_tags_modified());
#endif

        if ((ctx->exit_status = script_get_status(ctx->script)) != 0) {
            ctx->should_exit = TRUE;
        } else {
            animate(ctx);
            display(ctx); /* fps limiting done here */
            advance(ctx);
        }
    }
}
 
plist_action_id animate_particle(struct particle* p, void* userdata) {
    drawer_add_particle(((struct global_ctx*)userdata)->drawer, p);
    particle_tick(p);
    return particle_is_alive(p) ? ACTION_NEXT : ACTION_REMOVE;
}

void animate(struct global_ctx* ctx) {
    if (!ctx->paused) {
        plist_foreach(ctx->particles, animate_particle, ctx);
    }
}

void display(struct global_ctx* ctx) {
    /* fps limiting done in the drawer */
    if (!ctx->paused) {
        drawer_draw_to_screen(ctx->drawer);
    } else {
        drawer_preserve_screen(ctx->drawer);
    }
}

void advance(struct global_ctx* ctx) {
    static int delayctr = 0;
    if (ctx->args->interactive) {
        if (++delayctr % VIS_CMD_DELAY_NSTEPS == 0) {
            command_async(ctx->cmds);
            delayctr = 0;
        }
    }
    if (command_should_exit(ctx->cmds)) {
        ctx->should_exit = TRUE;
        ctx->exit_status = command_get_error(ctx->cmds);
    }
    if (!ctx->paused) {
        emitter_tick();
    }
}

