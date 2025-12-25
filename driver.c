#include "defines.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "audio.h"
#include "clargs.h"
#include "command.h"
#include "drawer.h"
#include "emit.h"
#include "emitter.h"
#include "helper.h"
#include "plist.h"
#include "plimits.h"
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

static void mainloop(struct global_ctx* ctx);
static void animate(struct global_ctx* ctx);
static void display(struct global_ctx* ctx);
static void advance(struct global_ctx* ctx);
static void onkeydown(int key, struct global_ctx* ctx);
static plist_action_id animate_particle(struct particle* p, void* userdata);

const char* get_key_name(int key) {
    static char buf[32];
    const char* name = glfwGetKeyName(key, 0);
    if (name) {
        return name;
    }

    switch (key) {
    case GLFW_KEY_ESCAPE:
        return "Escape";
    case GLFW_KEY_SPACE:
        return "Space";
    case GLFW_KEY_ENTER:
        return "Return";
    case GLFW_KEY_BACKSPACE:
        return "Backspace";
    case GLFW_KEY_UP:
        return "Up";
    case GLFW_KEY_DOWN:
        return "Down";
    case GLFW_KEY_LEFT:
        return "Left";
    case GLFW_KEY_RIGHT:
        return "Right";
    /* Add more as needed */
    default:
        snprintf(buf, sizeof(buf), "Key_%d", key);
        return buf;
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    struct global_ctx* ctx = (struct global_ctx*)glfwGetWindowUserPointer(window);
    if (ctx->args->debug_level > DEBUG_NONE) {
        DBPRINTF("Key %d (%s) scancode %d action %d mods %d", key, get_key_name(key),
                scancode, action, mods);
    }

    if (action == GLFW_PRESS) {
        onkeydown(key, ctx);
        script_keydown(ctx->script, get_key_name(key), mods & GLFW_MOD_SHIFT);
    } else if (action == GLFW_RELEASE) {
        script_keyup(ctx->script, get_key_name(key), mods & GLFW_MOD_SHIFT);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    struct global_ctx* ctx = (struct global_ctx*)glfwGetWindowUserPointer(window);
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    UNUSED_VARIABLE(mods);

    if (ctx->args->debug_level > DEBUG_NONE) {
        DBPRINTF("Mouse %d action %d mods %d at %g %g", button, action, mods, x, y);
    }

    if (action == GLFW_PRESS) {
        drawer_begin_trace(ctx->drawer);
        drawer_trace(ctx->drawer, (float)x, (float)y);
        script_mousedown(ctx->script, (int)x, (int)y, button);
    } else if (action == GLFW_RELEASE) {
        drawer_end_trace(ctx->drawer);
        script_mouseup(ctx->script, (int)x, (int)y, button);
    }
}

void cursor_position_callback(GLFWwindow* window, double x, double y) {
    struct global_ctx* ctx = (struct global_ctx*)glfwGetWindowUserPointer(window);
    if (ctx->args->debug_level >= DEBUG_DEBUG) {
        DBPRINTF("Mouse move x %g y %g", x, y);
    }
    drawer_trace(ctx->drawer, (float)x, (float)y);
    script_mousemove(ctx->script, (int)x, (int)y);
}

void cursor_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    struct global_ctx* ctx = (struct global_ctx*)glfwGetWindowUserPointer(window);
    if (ctx->args->debug_level > DEBUG_NONE) {
        DBPRINTF("Mouse scroll: xoffset %f yoffset %f", xoffset, yoffset);
    }
    drawer_trace_scroll(ctx->drawer, (float)xoffset, (float)yoffset);
    script_mousescroll(ctx->script, (int)xoffset, (int)yoffset);
}

int main(int argc, char* argv[]) {
    srand((unsigned)time(NULL));

    struct global_ctx g;
    ZEROINIT(&g);

    g.args = argparse(argc, argv);
    if (!g.args) {
        exit(1);
    } else if (g.args->must_exit) {
        int status = g.args->exit_status;
        clargs_free(g.args);
        exit(status);
    }

    plimits_update_screen_size(g.args->window_size[0], g.args->window_size[1]);

    g.drawer = drawer_new(g.args);
    if (!g.drawer) {
        exit(1);
    }
    drawer_config(g.drawer, g.args);

    /* Set window user pointer to global context and register callbacks */
    GLFWwindow* window = drawer_get_window(g.drawer);
    glfwSetWindowUserPointer(window, &g);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, cursor_scroll_callback);

    g.particles = plist_new(VIS_PLIST_INITIAL_SIZE);

    script_cfg_mask mask = SCRIPT_ALLOW_ALL;
    if (g.args->stay_after_script) {
        mask |= SCRIPT_NO_EXIT;
    }
    g.script = script_new(mask, g.args);
    if (klist_length(g.args->scriptargs) > 0) {
        script_set_args(g.script, g.args->scriptargs);
    } else {
        klist_free(g.args->scriptargs);
        g.args->scriptargs = NULL;
    }
    script_set_drawer(g.script, g.drawer);

    g.cmds = command_setup(g.drawer, g.particles, g.script, g.args->interactive);

    emitter_setup(g.cmds, g.particles, g.drawer);

    if (!audio_init()) {
        exit(1);
    }
    if (g.args->quiet_audio) {
        audio_mute();
    }

    /* Configure particles drawn when the user clicks and drags */
    emit_desc* emit = emit_new();
    emit->n = 100;
    emit->rad = 1;
    emit_set_life(emit, 100, 200);
    emit_set_ds(emit, 0.2, 0.2);
    emit_set_angle(emit, 0, 2 * M_PI);
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

    clargs_free(g.args);
    drawer_free(g.drawer);
    plist_free(g.particles);
    command_teardown(g.cmds);
    emitter_free();
    audio_free();
    script_free(g.script);

    return g.exit_status;
}

void onkeydown(int sym, struct global_ctx* ctx) {
    switch (sym) {
    case GLFW_KEY_ESCAPE:
        ctx->should_exit = TRUE;
        break;
    case GLFW_KEY_SPACE:
        ctx->paused = !ctx->paused;
        if (ctx->paused) {
            audio_pause();
        } else {
            audio_play();
        }
        break;
    default:
        break;
    }
}

void mainloop(struct global_ctx* ctx) {
    GLFWwindow* window = drawer_get_window(ctx->drawer);

    while (!ctx->should_exit && !glfwWindowShouldClose(window)) {
        glfwPollEvents();

        struct script_debug dbg;
        script_get_debug(ctx->script, &dbg);

        script_set_debug(ctx->script, SCRIPT_DEBUG_FRAMES_EMITTED,
            emitter_get_frame_count(VIS_FTYPE_EMIT));
        script_set_debug(
            ctx->script, SCRIPT_DEBUG_TIME_NOW, (uint32_t)(glfwGetTime() * 1000.0));
        script_set_debug(ctx->script, SCRIPT_DEBUG_NUM_MUTATES,
            emitter_get_frame_count(VIS_FTYPE_MUTATE));
        script_set_debug(ctx->script, SCRIPT_DEBUG_PARTICLES_EMITTED,
            dbg.particles_emitted + plist_get_size(ctx->particles));
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
        ctx->exit_status = (int)command_get_error(ctx->cmds);
    }
    if (!ctx->paused) {
        emitter_tick();
    }
}
