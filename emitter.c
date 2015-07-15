
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

static struct emitter {
    struct commands* commands;
    plist_t particles;
    flist_t fl;
    drawer_t drawer;
    ftype_id frame_counts[VIS_MAX_FTYPE];
} emitter;

void emitter_setup(struct commands* cmds, plist_t plist, drawer_t drawer) {
    ZEROINIT(&emitter);
    emitter.commands = cmds;
    emitter.particles = plist;
    emitter.drawer = drawer;
    for (ftype_id i = (ftype_id)0; i < VIS_MAX_FTYPE; ++i) {
        emitter.frame_counts[i] = (ftype_id)0;
    }
}

void emitter_free(UNUSED_PARAM(void* arg)) {
    if (emitter.fl) {
        flist_clear(emitter.fl);
        flist_free(emitter.fl);
    }
}

uint32_t emitter_get_frame_count(ftype_id ft) {
    return emitter.frame_counts[ft];
}

void emitter_schedule(flist_t frames) {
    if (emitter.fl && emitter.fl != frames) {
        flist_clear(emitter.fl);
        flist_free(emitter.fl);
    } else if (emitter.fl == frames) {
        flist_restart(emitter.fl);
    } else {
        emitter.fl = frames;
    }
}

static plist_action_id do_mutate_fn(struct particle* p,
                                   UNUSED_PARAM(size_t idx),
                                   void* mutate) {
    mutate_method_t method = mutate;
    method->func(p, method);
    return ACTION_NEXT;
}

void emitter_tick(void) {
    flist_node_t fn = flist_tick(emitter.fl);
    while (fn != NULL) {
        emitter.frame_counts[fn->type] += 1;
        switch (fn->type) {
            case VIS_FTYPE_EMIT:
                emit_frame(fn->data.frame);
                break;
            case VIS_FTYPE_EXIT:
                command_str(emitter.commands, "exit");
                break;
            case VIS_FTYPE_PLAY:
                audio_play();
                break;
            case VIS_FTYPE_CMD:
                command_str(emitter.commands, fn->data.cmd);
                break;
            case VIS_FTYPE_BGCOLOR:
                drawer_bgcolor(emitter.drawer, fn->data.color[0],
                               fn->data.color[1], fn->data.color[2]);
                break;
            case VIS_FTYPE_MUTATE:
                plist_foreach(emitter.particles, do_mutate_fn,
                              fn->data.method);
                break;
            case VIS_FTYPE_SCRIPTCB:
                script_run_cb(fn->data.scriptcb->owner, fn->data.scriptcb,
                              NULL);
                break;
            case VIS_FTYPE_FRAMESEEK:
                flist_goto_frame(emitter.fl, fn->data.frameseek);
                break;
            case VIS_MAX_FTYPE:
                break;
        }
        /* do not process next node if this is a frame seek */
        fn = fn->type != VIS_FTYPE_FRAMESEEK ? flist_node_next(fn) : NULL;
    }
}

void emit_frame(emit_t frame) {
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
        plist_add(emitter.particles, p);
    }
}

