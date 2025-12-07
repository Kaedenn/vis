
#include "emitter.h"

#include "audio.h"
#include "command.h"
#include "helper.h"
#include "particle.h"
#include "pextra.h"

#include "drawer.h"

#include "random.h"

#include <string.h>

static struct emitter {
    struct commands* commands;
    plist_t particles;
    flist* fl;
    drawer_t drawer;
    uint32_t frame_counts[VIS_MAX_FTYPE];
} emitter;

void emitter_setup(struct commands* cmds, plist_t plist, drawer_t drawer) {
    ZEROINIT(&emitter);
    emitter.commands = cmds;
    emitter.particles = plist;
    emitter.drawer = drawer;
    for (ftype_id i = (ftype_id)0; i < VIS_MAX_FTYPE; ++i) {
        emitter.frame_counts[i] = 0;
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

void emitter_schedule(flist* frames) {
    if (emitter.fl == NULL) {
        emitter.fl = frames;
    } else if (emitter.fl && emitter.fl != frames) {
        flist_clear(emitter.fl);
        flist_free(emitter.fl);
    } else if (emitter.fl == frames) {
        flist_restart(emitter.fl);
    } else {
        VIS_ASSERT(!(BOOL)(long)"unreachable");
    }
}

static plist_action_id do_mutate_fn(particle* p, void* mutate) {
    ((mutate_method*)mutate)->func(p, mutate);
    return ACTION_NEXT;
}

void emitter_tick(void) {
    flist_node* fn = flist_tick(emitter.fl);
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
            drawer_bgcolor(
                emitter.drawer, fn->data.color[0], fn->data.color[1], fn->data.color[2]);
            break;
        case VIS_FTYPE_MUTATE:
            plist_foreach(emitter.particles, do_mutate_fn, fn->data.method);
            break;
        case VIS_FTYPE_SCRIPTCB:
            script_run_cb(fn->data.scriptcb->owner, fn->data.scriptcb, NULL);
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

void emit_frame(emit_desc* frame) {
    for (int i = 0; i < frame->n; ++i) {
        particle* p = NULL;
        float r = randfloat(frame->r - frame->ur, frame->r + frame->ur);
        float g = randfloat(frame->g - frame->ug, frame->g + frame->ug);
        float b = randfloat(frame->b - frame->ub, frame->b + frame->ub);
        pextra* pe = new_pextra(r, g, b, frame->blender);
        p = particle_new_full(frame->x, frame->y, frame->ux, frame->uy, frame->rad,
            frame->urad, frame->ds, frame->uds, frame->theta, frame->utheta, frame->life,
            frame->ulife, frame->force, frame->limit, pe);
        plist_add(emitter.particles, p);
    }
}
