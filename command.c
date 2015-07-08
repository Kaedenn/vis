
#include "command.h"

#include "async.h"
#include "audio.h"
#include "defines.h"
#include "drawer.h"
#include "emitter.h"
#include "forces.h"
#include "helper.h"
#include "particle.h"
#include "particle_extra.h"
#include "plimits.h"
#include "random.h"
#include "script.h"

/* for M_PI */
#define __USE_BSD

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

struct commands {
    drawer_t drawer;
    plist_t particles;
    script_t script;
    BOOL should_exit;
    cmd_error_id exit_status;
    BOOL interactive;
};

static void cmd_emit(struct commands* cmds, const char* buffer);
static void cmd_kick(struct commands* cmds, const char* buffer);
static void cmd_snare(struct commands* cmds, const char* buffer);
static void cmd_strum(struct commands* cmds, const char* buffer);
static void cmd_rain(struct commands* cmds, const char* buffer);
static void cmd_load(struct commands* cmds, const char* buffer);
static void cmd_lua(struct commands* cmds, const char* buffer);
static void cmd_audio(struct commands* cmds, const char* buffer);
static void cmd_exit(struct commands* cmds, const char* buffer);
static void cmd_help(struct commands* cmds, const char* buffer);

static struct cmd_map {
    const char* cmd;
    void (*func)(struct commands* cmds, const char* buffer);
    const char* synopsis;
} commands[] = {
    {"emit", cmd_emit, "instant emit: type \"help emit\" to see args"},
    {"kick", cmd_kick, "takes one arg: number of particles"},
    {"snare", cmd_snare, "takes one arg: number of particles"},
    {"strum", cmd_strum, "takes one arg: number of particles"},
    {"rain", cmd_rain, "takes one arg: number of particles"},
    {"load", cmd_load, "takes one arg: scriptfile.lua path"},
    {"lua", cmd_lua, "runs the script specified as \"lua <script>\""},
    {"audio", cmd_audio, "takes one arg: filename.wav path"},
    {"exit", cmd_exit, "exits simulation immediately"},
    {"help", cmd_help, "briefly describes commands"},
    {NULL, NULL, NULL}
};

struct commands* command_setup(drawer_t drawer, plist_t plist,
                               script_t script, BOOL interactive) {
    struct commands* cmds = DBMALLOC(sizeof(struct commands));
    cmds->drawer = drawer;
    cmds->particles = plist;
    cmds->script = script;
    cmds->interactive = interactive;
    if (interactive) {
        async_setup_stdin();
        async_setup_stdout();
        async_write_stdout(">>> ");
    }
    return cmds;
}

void command_teardown(struct commands* cmds) {
    if (cmds->interactive) {
        async_write_stdout("\n");
    }
    DBFREE(cmds);
}

BOOL command_should_exit(struct commands* cmds) {
    return cmds->should_exit;
}

cmd_error_id command_get_error(struct commands* cmds) {
    return cmds->exit_status;
}


/*
nice strums:
emit 1000 400 300 0 0 3 1 1 0.99 1.57 3.14 90 10 0 1.5 1.5 0 0 0 0 1
emit 1000 400 0 0 0 3 1 2 1.99 1.57 3.14 90 10 0 1.5 1.5 0 0 0 1 1
*/

void command(struct commands* cmds) {
    static char buffer[VIS_BUFFER_LEN];
    buffer[0] = '\0';
    ssize_t bytes = async_read_stdin(buffer, VIS_BUFFER_LEN);
    if (bytes == -1 && errno != EAGAIN) {
        int err = errno;
        const char* errstr = strerror(errno);
        eprintf("received error %d: %s from async_read_stdin",
                err, errstr);
        cmds->should_exit = TRUE;
        cmds->exit_status = CMD_ERROR_FATAL;
    } else if (bytes == 0) {
        command_str(cmds, "exit");
    } else if (buffer[0] != '\0') { /* something was read */
        if (buffer[0] != '\n') { /* and the line wasn't empty */
            command_str(cmds, buffer);
        }
        async_write_stdout(">>> ");
    }
}

void command_str(struct commands* cmds, const char* buffer) {
    static char rw_buffer[1024];
    int i = 0;
    char* c;
    if (strlen(buffer) > 1024) {
        eprintf("Refusing to handle insane command %p", buffer);
        return;
    }
    strcpy(rw_buffer, buffer);
    if ((c = strchr(rw_buffer, '\n')) != NULL) {
        *c = '\0';
    }
    while (commands[i].cmd != NULL) {
        if (startswith(rw_buffer, commands[i].cmd)) {
            (*commands[i].func)(cmds, rw_buffer);
            break;
        }
        ++i;
    }
    if (commands[i].cmd == NULL) {
        eprintf("Unrecognized command");
    }
}

static void cmd_emit(struct commands* cmds, const char* buffer) {
    static const int nargs = 22;
    int n;
    double x, y, ux, uy;
    double rad, urad;
    double ds, uds, theta, utheta;
    int life, ulife;
    float r, g, b;        /* particle color expected value */
    float ur, ug, ub; /* particle color std. dev. */
    float ar, ag, ab; /* actual particle color */
    int force;
    int limit;
    int blender;
    int i = 0;
    if (sscanf(buffer, "emit %d %lg %lg %lg %lg "
               "%lg %lg %lg %lg %lg %lg "
               "%d %d %g %g %g %g %g %g "
               "%d %d %d",
               &n,     &x,     &y,  &ux,  &uy,
               &rad,   &urad,  &ds, &uds, &theta, &utheta,
               &life,  &ulife, &r, &g, &b, &ur, &ug, &ub,
               &force, &limit, &blender) == nargs) {
        for (i = 0; i < n; ++i) {
            pextra_t pe = NULL;
            struct particle* p = NULL;
            ar = randfloat(r-ur, r+ur);
            ag = randfloat(g-ug, g+ug);
            ab = randfloat(b-ub, b+ub);
            pe = new_particle_extra(ar, ag, ab, blender);
            
            if (force < 0 || force >= VIS_NFORCES) {
                force = VIS_DEFAULT_FORCE;
            }
            if (limit < 0 || limit >= VIS_NLIMITS) {
                limit = VIS_DEFAULT_LIMIT;
            }
            
            p = particle_new_full(x, y, ux, uy, rad, urad, ds, uds,
                                  theta, utheta, life, ulife,
                                  force, limit, pe);
            
            plist_add(cmds->particles, p);
        }
    } else {
        eprintf("usage: emit n x y ux uy rad urad ds uds theta utheta "
                "life ulife r g b ur ug ub force limit blender");
        eprintf("(type 'help emit' for explanation)");
    }
}

static void cmd_kick(struct commands* cmds, const char* buffer) {
    static const int nargs = 1;
    static const double x = VIS_WIDTH / 2;
    static const double y = VIS_HEIGHT / 2;
    int i = 0;
    int arg = 0;
    if (sscanf(buffer, "kick %d", &arg) == nargs) {
        pextra_t pe = NULL;
        struct particle* p = NULL;
        float r, g, b;
        int radius, life;
        double ds, theta;
        while (i < arg) {
            radius = randint(1, 3);
            life = randint(VIS_FPS_LIMIT, 2 * VIS_FPS_LIMIT);
            r = randfloat(1.0f, 2.0f);
            g = randfloat(0.0f, 0.7f);
            b = randfloat(0.0f, 0.4f);
            pe = new_particle_extra(r, g, b, VIS_BLEND_LINEAR);
            p = particle_new(x, y, radius, life, pe);
            ds = randdouble(0.1, 3.0);
            theta = randdouble(0.0, 2*M_PI);
            particle_push(p, ds*cos(theta), ds*sin(theta));
            particle_set_force(p, VIS_FORCE_FRICTION);
            particle_set_limit(p, VIS_LIMIT_SPRINGBOX);
            plist_add(cmds->particles, p);
            ++i;
        }
    } else {
        eprintf("usage: kick <#particles>");
    }
}

static void cmd_snare(struct commands* cmds, const char* buffer) {
    static const int nargs = 1;
    static const double x = VIS_WIDTH / 2;
    static const double y = VIS_HEIGHT / 2;
    int i = 0;
    int arg = 0;
    if (sscanf(buffer, "snare %d", &arg) == nargs) {
        pextra_t pe = NULL;
        struct particle* p = NULL;
        float r, g, b;
        int radius, life;
        double ds, theta;
        while (i < arg) {
            radius = randint(1, 3);
            life = randint(VIS_FPS_LIMIT, 2 * VIS_FPS_LIMIT);
            r = randfloat(0.0f, 0.4f);
            g = randfloat(0.0f, 1.0f);
            b = randfloat(0.0f, 0.4f);
            pe = new_particle_extra(r, g, b, VIS_BLEND_LINEAR);
            p = particle_new(x, y, radius, life, pe);
            ds = randdouble(0.1, 2.0);
            theta = randdouble(0.0, 2*M_PI);
            particle_push(p, ds*cos(theta), ds*sin(theta));
            particle_set_force(p, VIS_FORCE_GRAVITY);
            particle_set_limit(p, VIS_LIMIT_SPRINGBOX);
            plist_add(cmds->particles, p);
            ++i;
        }
    } else {
        eprintf("usage: snare <#particles>");
    }
}

static void cmd_strum(struct commands* cmds, const char* buffer) {
    static const int nargs = 1;
    static const double x = VIS_WIDTH / 2;
    static const double y = VIS_HEIGHT;
    int i = 0;
    int arg = 0;
    if (sscanf(buffer, "strum %d", &arg) == nargs) {
        pextra_t pe = NULL;
        struct particle* p = NULL;
        float r, g, b;
        double radius;
        int life;
        double ds, theta;
        while (i < arg) {
            radius = randdouble(1.0, 2.0);
            life = randint(VIS_FPS_LIMIT, 2 * VIS_FPS_LIMIT);
            r = randfloat(0.0f, 0.4f);
            g = randfloat(0.0f, 0.7f);
            b = randfloat(0.0f, 1.0f);
            pe = new_particle_extra(r, g, b, VIS_BLEND_LINEAR);
            p = particle_new(x, y, radius, life, pe);
            ds = randdouble(0.1, 3.0);
            theta = randdouble(0.0, 2*M_PI);
            particle_push(p, ds*cos(theta), ds*sin(theta));
            particle_set_force(p, VIS_FORCE_FRICTION);
            particle_set_limit(p, VIS_LIMIT_SPRINGBOX);
            plist_add(cmds->particles, p);
            ++i;
        }
    } else {
        eprintf("usage: strum <#particles>");
    }
}

static void cmd_rain(struct commands* cmds, const char* buffer) {
    static const int nargs = 1;
    int i = 0;
    int arg = 0;
    if (sscanf(buffer, "rain %d", &arg) == nargs) {
        pextra_t pe = NULL;
        struct particle* p = NULL;
        int x, y;
        float r, g, b;
        double radius;
        int life;
        double ds, theta;
        while (i < arg) {
            x = randint(0, VIS_WIDTH);
            y = 0;
            radius = randdouble(0.5, 1.5);
            life = randint(VIS_FPS_LIMIT * 5, VIS_FPS_LIMIT / 5);
            r = 0.0f;
            g = randfloat(0.1f, 0.3f);
            b = randfloat(0.4f, 1.0f);
            pe = new_particle_extra(r, g, b, VIS_BLEND_NONE);
            p = particle_new(x, y, radius, life, pe);
            ds = randdouble(0.0, 1.0);
            theta = M_PI * 3 / 2;
            particle_push(p, ds*cos(theta), ds*sin(theta));
            particle_set_force(p, VIS_FORCE_GRAVITY);
            particle_set_limit(p, VIS_DEFAULT_LIMIT);
            plist_add(cmds->particles, p);
            ++i;
        }
    } else {
        eprintf("usage: rain <#particles>");
    }
}

static void cmd_load(struct commands* cmds, const char* buffer) {
    if (strlen(buffer) > strlen("load ")) {
        flist_t flist = script_run(cmds->script, buffer + strlen("load "));
        if (flist != NULL) {
            emitter_schedule(flist);
        } else {
            eprintf("Failed to load script '%s'", buffer + strlen("load "));
        }
    } else {
        eprintf("usage: load <script-path>");
    }
}

static void cmd_lua(struct commands* cmds, const char* buffer) {
    if (strlen(buffer) > strlen("lua ")) {
        script_run_string(cmds->script, buffer + strlen("lua "));
    } else {
        eprintf("usage: lua <script...>");
    }
}

static void cmd_audio(UNUSED_PARAM(struct commands* cmds), const char* buffer) {
    if (strlen(buffer) > strlen("audio ")) {
        audio_open(buffer + strlen("audio "));
    } else {
        eprintf("usage: audio <path>");
    }
}

static void cmd_exit(UNUSED_PARAM(struct commands* cmds),
                     UNUSED_PARAM(const char* buffer)) {
    /* depends on the gc freeing everything else */
    cmds->should_exit = TRUE;
    cmds->exit_status = CMD_ERROR_NONE;
}

static void cmd_help(UNUSED_PARAM(struct commands* cmds), const char* buffer) {
    size_t i = 0;
    if (startswith(buffer, "help emit")) {
        const char* help[] = {
"instantaneous emit command (please see README.md):\n",
"emit n x y ux uy rad urad ds uds theta utheta life ulife r g b ur ug ub force "
    "limit blender\n",
"\tx = random(x-ux, x+ux); u<val> is \"uncertainty\" or variance for <val>\n",
"\tn, life, ulife, force, limit, and blender are all integers\n",
"\ttheta is between 0 and 2*PI\n",
"\tlife is measured in frames (30 per second), so 60 is two seconds\n",
"\tr, g, b are between 0 and 255\n",
"\tforces: 0=nothing, 1=friction 2=gravity\n",
"\tlimits: 0=nothing, 1=box (stop at edge), 2=springbox (bounce off edge)\n",
"\tblender: 0=nothing, 1=linear (fade to black), 2=quadratic, 3=ease-out\n",
NULL
        };
        for (i = 0; help[i]; ++i) {
            printf("%s", help[i]);
        }
    } else {
        for (i = 0; commands[i].cmd != NULL; ++i) {
            printf("Command: \"%s\": %s\n", commands[i].cmd, commands[i].synopsis);
        }
    }
}

