
#include "emitter.h"

#include "audio.h"
#include "command.h"
#include "helper.h"
#include "particle.h"
#include "particle_extra.h"

#include "forces.h"
#include "plimits.h"
#include "drawer.h"

#include "random.h"

#include <string.h>

/* FIXME: move all these to a heap-allocated structure */
static struct commands* commands;
static plist_t particles = NULL;
static flist_t fl = NULL;
static uint32_t emit_frame_count = 0;
static uint32_t num_mutates = 0;

void emitter_setup(struct commands* cmds, plist_t plist) {
    commands = cmds;
    particles = plist;
}

void emitter_free(UNUSED_PARAM(void* arg)) {
}

uint32_t emitter_get_emit_frame_count(void) {
    return emit_frame_count;
}

uint32_t emitter_get_num_mutates(void) {
    return num_mutates;
}

void emitter_schedule(flist_t frames) {
    if (fl) {
        flist_clear(fl);
        flist_free(fl);
    }
    fl = frames;
}

static plist_action_t do_mutate_fn(struct particle* p, UNUSED_PARAM(size_t idx),
                                   void* mutate) {
    mutate_method_t method = mutate;
    method->func(p, method->factor);
    return ACTION_NEXT;
}

void emitter_tick(void) {
    flist_node_t fn = flist_tick(fl);
    while (fn != NULL) {
        switch (fn->type) {
            case VIS_FTYPE_EMIT:
                emit_frame(fn->data.frame);
                break;
            case VIS_FTYPE_EXIT:
                DBPRINTF("received command %s", "exit");
                command_str(commands, "exit");
                break;
            case VIS_FTYPE_CMD:
                command_str(commands, fn->data.cmd);
                break;
            case VIS_FTYPE_BGCOLOR:
                eprintf("No longer implemented, %s", "sorry!");
                break;
            case VIS_FTYPE_MUTATE:
                num_mutates += 1;
                plist_foreach(particles, do_mutate_fn, fn->data.method);
                break;
            case VIS_FTYPE_SCRIPTCB:
                call_script(fn->data.scriptcb->owner, fn->data.scriptcb, NULL);
                break;
            case VIS_FTYPE_FRAMESEEK:
                flist_goto_frame(fl, fn->data.frameseek);
            case VIS_MAX_FTYPE:
            default:
                break;
        }
        fn = flist_node_next(fn);
    }
}

void emit_frame(emit_t frame) {
    emit_frame_count += 1;
    for (int i = 0; i < frame->n; ++i) {
        struct particle* p = NULL;
        pextra_t pe = NULL;
        float r, g, b;
        r = randfloat(frame->r - frame->ur, frame->r + frame->ur);
        g = randfloat(frame->g - frame->ug, frame->g + frame->ug);
        b = randfloat(frame->b - frame->ub, frame->b + frame->ub);
        pe = new_particle_extra(r, g, b, frame->blender);
        p = particle_new_full(frame->x, frame->y, frame->ux, frame->uy,
                              frame->rad, frame->urad, frame->ds, frame->uds,
                              frame->theta, frame->utheta,
                              frame->life, frame->ulife,
                              frame->force, frame->limit, pe);
        plist_add(particles, p);
    }
}

