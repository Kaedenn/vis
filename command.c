
#include "command.h"

#include "async.h"
#include "defines.h"
#include "draw.h"
#include "emitter.h"
#include "forces.h"
#include "frame.h"
#include "helper.h"
#include "limits.h"
#include "particle.h"
#include "particle_extra.h"
#include "script.h"

#include "random.h"

/* for M_PI */
#define __USE_BSD

#include <math.h>
#include <stdio.h>
#include <string.h>

#define NCOMMANDS 9

static plist_t particles = NULL;

static void cmd_emit(const char* buffer);
static void cmd_kick(const char* buffer);
static void cmd_snare(const char* buffer);
static void cmd_strum(const char* buffer);
static void cmd_rain(const char* buffer);
static void cmd_load(const char* buffer);
static void cmd_exit(const char* buffer);
static void cmd_bgc(const char* buffer);

static struct cmd_map {
    const char* cmd;
    void (*func)(const char* buffer);
} commands[NCOMMANDS] = {
    {"emit", cmd_emit},
    {"kick", cmd_kick},
    {"snare", cmd_snare},
    {"strum", cmd_strum},
    {"rain", cmd_rain},
    {"load", cmd_load},
    {"exit", cmd_exit},
    {"bgc", cmd_bgc}
};

void command_setup(plist_t plist) {
    particles = plist;
    async_setup_stdin();
    async_setup_stdout();
    async_write_stdout(">>> ");
}

/*

nice strums:
emit 1000 400 300 0 0 3 1 1 0.99 1.57 3.14 90 10 0 1.5 1.5 0 0 0 0 1
emit 1000 400 0 0 0 3 1 2 1.99 1.57 3.14 90 10 0 1.5 1.5 0 0 0 1 1

*/

void command(void) {
    static char buffer[VIS_BUFFER_LEN];
    buffer[0] = '\0';
    async_read_stdin(buffer, VIS_BUFFER_LEN);
    if (buffer[0] != '\0') { /* something was read */
        if (buffer[0] != '\n') { /* and the line wasn't empty */
            docommand(buffer);
        }
        async_write_stdout(">>> ");
    }
}

void docommand(const char* buffer) {
    int i = 0;
    if (strchr(buffer, '\n')) {
        *strchr(buffer, '\n') = '\0';
    }
    while (i < NCOMMANDS) {
        if (startswith(buffer, commands[i].cmd)) {
            (*commands[i].func)(buffer);
            break;
        }
        ++i;
    }
    if (i == NCOMMANDS) {
        eprintf("Invalid input");
    }
}

static void cmd_emit(const char* buffer) {
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
            particle_t p = NULL;
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
            
            plist_add(particles, p);
        }
    } else {
        eprintf("bad command arguments");
    }
}

static void cmd_kick(const char* buffer) {
    static const int nargs = 1;
    static const double x = VIS_WIDTH / 2;
    static const double y = VIS_HEIGHT / 2;
    int i = 0;
    int arg = 0;
    if (sscanf(buffer, "kick %d", &arg) == nargs) {
        pextra_t pe = NULL;
        particle_t p = NULL;
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
            particle_set_force(p, VIS_FRICTION);
            particle_set_limit(p, VIS_SPRINGBOX);
            plist_add(particles, p);
            ++i;
        }
    } else {
        eprintf("bad command arguments");
    }
}

static void cmd_snare(const char* buffer) {
    static const int nargs = 1;
    static const double x = VIS_WIDTH / 2;
    static const double y = VIS_HEIGHT / 2;
    int i = 0;
    int arg = 0;
    if (sscanf(buffer, "snare %d", &arg) == nargs) {
        pextra_t pe = NULL;
        particle_t p = NULL;
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
            particle_set_force(p, VIS_GRAVITY);
            particle_set_limit(p, VIS_SPRINGBOX);
            plist_add(particles, p);
            ++i;
        }
    } else {
        eprintf("bad command arguments");
    }
}

static void cmd_strum(const char* buffer) {
    static const int nargs = 1;
    static const double x = VIS_WIDTH / 2;
    static const double y = VIS_HEIGHT;
    int i = 0;
    int arg = 0;
    if (sscanf(buffer, "strum %d", &arg) == nargs) {
        pextra_t pe = NULL;
        particle_t p = NULL;
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
            particle_set_force(p, VIS_FRICTION);
            particle_set_limit(p, VIS_SPRINGBOX);
            plist_add(particles, p);
            ++i;
        }
    } else {
        eprintf("bad command arguments");
    }
}

static void cmd_rain(const char* buffer) {
    static const int nargs = 1;
    int i = 0;
    int arg = 0;
    if (sscanf(buffer, "rain %d", &arg) == nargs) {
        pextra_t pe = NULL;
        particle_t p = NULL;
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
            pe = new_particle_extra(r, g, b, VIS_NO_BLEND);
            p = particle_new(x, y, radius, life, pe);
            ds = randdouble(0.0, 1.0);
            theta = M_PI * 3 / 2;
            particle_push(p, ds*cos(theta), ds*sin(theta));
            particle_set_force(p, VIS_GRAVITY);
            particle_set_limit(p, VIS_DEFAULT_LIMIT);
            plist_add(particles, p);
            ++i;
        }
    } else {
        eprintf("bad command arguments");
    }
}

static void cmd_load(const char* buffer) {
    if (strlen(buffer) > strlen("load ")) {
        flist_t flist = load_script(buffer + strlen("load "));
        if (flist != NULL) {
            emitter_schedule(flist);
        } else {
            eprintf("Failed to load script '%s'", buffer + strlen("load "));
        }
    }
}

static void cmd_exit(UNUSED_PARAM(const char* buffer)) {
    exit(0);
}

static void cmd_bgc(const char* buffer) {
    static const int nargs = 4;
    float r, g, b, a;
    if (sscanf(buffer, "bgc %f %f %f %f", &r, &g, &b, &a) == nargs) {
        set_background_color(r, g, b, a);
    }
}

