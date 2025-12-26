
#include "defines.h"

#include "emitter.h"

#include "audio.h"
#include "command.h"
#include "drawer.h"
#include "helper.h"
#include "particle.h"
#include "pextra.h"
#include "random.h"

#include <time.h>

static struct emitter {
    struct commands* commands;              /* pointer to commands object */
    plist_t particles;                      /* pointer to plist object */
    flist_t fl;                             /* pointer to flist object */
    drawer_t drawer;                        /* pointer to drawer object */
    uint32_t frame_counts[VIS_MAX_FTYPE];   /* frame type counts */
    uint32_t delay_counter;                 /* delay frames counter */
    BOOL do_sync;                           /* re-sync audio after delay */
} emitter;

void emitter_setup(struct commands* cmds, plist_t plist, drawer_t drawer) {
    ZEROINIT(&emitter);
    emitter.commands = cmds;
    emitter.particles = plist;
    emitter.drawer = drawer;
}

void emitter_free(void) {
    if (emitter.fl) {
        flist_clear(emitter.fl);
        flist_free(emitter.fl);
        emitter.fl = NULL;
    }
}

uint32_t emitter_get_frame_count(ftype_id ft) {
    VIS_ASSERT(ft >= 0 && ft <= VIS_MAX_FTYPE);
    return emitter.frame_counts[ft];
}

void emitter_schedule(flist_t frames) {
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
    if (emitter.delay_counter > 0) {
        DBPRINTF("Delaying for %d frame%s at frame %d", emitter.delay_counter,
            (emitter.delay_counter == 1 ? "" : "s"), emitter.fl->curr_frame);
        emitter.delay_counter -= 1;
        return;
    }
    if (emitter.do_sync) {
        DBPRINTF("%s", "Sync delay is over; restarting audio track");
        emitter.do_sync = FALSE;
        audio_seek(0U);
        audio_unmute();
        if (!audio_is_playing()) {
            audio_play();
        }
    }
    fnum curr_frame = emitter.fl->curr_frame;
    flist_node* fn = flist_tick(emitter.fl);
    while (fn != NULL) {
        const char* ftype_str = ftype_to_string(fn->type);
        emitter.frame_counts[fn->type] += 1;
        switch (fn->type) {
        case VIS_FTYPE_EMIT:
            /*DBPRINTF("Ticking flist: %s at frame %d", ftype_str, curr_frame);*/
            emit_frame(fn->data.frame);
            break;
        case VIS_FTYPE_EXIT:
            DBPRINTF("Ticking flist: %s at frame %d", ftype_str, curr_frame);
            command_str(emitter.commands, "exit");
            break;
        case VIS_FTYPE_PLAY:
            DBPRINTF("Ticking flist: %s at frame %d", ftype_str, curr_frame);
            audio_play();
            break;
        case VIS_FTYPE_CMD:
            DBPRINTF("Ticking flist: %s at frame %d", ftype_str, curr_frame);
            command_str(emitter.commands, fn->data.cmd);
            break;
        case VIS_FTYPE_BGCOLOR:
            DBPRINTF("Ticking flist: %s at frame %d", ftype_str, curr_frame);
            drawer_bgcolor(emitter.drawer,
                    fn->data.color[0], fn->data.color[1], fn->data.color[2]);
            break;
        case VIS_FTYPE_MUTATE:
            DBPRINTF("Ticking flist: %s at frame %d", ftype_str, curr_frame);
            plist_foreach(emitter.particles, do_mutate_fn, fn->data.method);
            break;
        case VIS_FTYPE_SCRIPTCB:
            DBPRINTF("Ticking flist: %s %s %s at frame %d",
                    ftype_str,
                    fn->data.scriptcb->fn_name,
                    fn->data.scriptcb->fn_code,
                    curr_frame);
            script_run_cb(fn->data.scriptcb->owner, fn->data.scriptcb, NULL);
            break;
        case VIS_FTYPE_FRAMESEEK:
            DBPRINTF("Ticking flist: %s to %d at %d",
                    ftype_str, fn->data.frameseek, curr_frame);
            flist_goto_frame(emitter.fl, fn->data.frameseek);
            break;
        case VIS_FTYPE_DELAY:
            DBPRINTF("Ticking flist: %s %d at frame %d",
                    ftype_str, fn->data.delay, curr_frame);
            emitter.delay_counter = fn->data.delay;
            emitter.do_sync = FALSE;
            break;
        case VIS_FTYPE_AUDIOSYNC:
            DBPRINTF("Ticking flist: %s %d at frame %d",
                    ftype_str, fn->data.delay, curr_frame);
            emitter.delay_counter = fn->data.delay;
            emitter.do_sync = TRUE;
            audio_mute();
            if (audio_is_playing()) {
                audio_pause();
            }
            break;
        case VIS_MAX_FTYPE:
            break;
        }
        /* frame seeks are immediate; terminate processing accordingly */
        fn = (fn->type == VIS_FTYPE_FRAMESEEK) ? NULL : flist_node_next(fn);
    }
}

void emit_frame(emit_desc* frame) {
    for (int i = 0; i < frame->n; ++i) {
        float r = frame->r;
        float g = frame->g;
        float b = frame->b;
        if (frame->ur != 0.0f) {
            r = randfloat(frame->r - frame->ur, frame->r + frame->ur);
        }
        if (frame->ug != 0.0f) {
            g = randfloat(frame->g - frame->ug, frame->g + frame->ug);
        }
        if (frame->ub != 0.0f) {
            b = randfloat(frame->b - frame->ub, frame->b + frame->ub);
        }
        plist_add(emitter.particles, particle_new_circle(
                frame->x, frame->y, frame->ux, frame->uy,
                frame->s, frame->us,
                frame->rad, frame->urad,
                frame->ds, frame->uds,
                frame->theta, frame->utheta,
                frame->life, frame->ulife,
                frame->force, frame->limit,
                new_pextra(r, g, b, frame->blender)));
    }
}
