
#include "flist.h"
#include "helper.h"
#include "script.h"

const char* ftype_to_string(ftype_id ftype) {
    switch (ftype) {
    case VIS_FTYPE_EMIT:
        return "VIS_FTYPE_EMIT";
    case VIS_FTYPE_EXIT:
        return "VIS_FTYPE_EXIT";
    case VIS_FTYPE_PLAY:
        return "VIS_FTYPE_PLAY";
    case VIS_FTYPE_PAUSE:
        return "VIS_FTYPE_PAUSE";
    case VIS_FTYPE_VOLUME:
        return "VIS_FTYPE_VOLUME";
    case VIS_FTYPE_AUDIOSEEK:
        return "VIS_FTYPE_AUDIOSEEK";
    case VIS_FTYPE_CMD:
        return "VIS_FTYPE_CMD";
    case VIS_FTYPE_BGCOLOR:
        return "VIS_FTYPE_BGCOLOR";
    case VIS_FTYPE_MUTATE:
        return "VIS_FTYPE_MUTATE";
    case VIS_FTYPE_SCRIPTCB:
        return "VIS_FTYPE_SCRIPTCB";
    case VIS_FTYPE_FRAMESEEK:
        return "VIS_FTYPE_FRAMESEEK";
    case VIS_FTYPE_DELAY:
        return "VIS_FTYPE_DELAY";
    case VIS_FTYPE_AUDIOSYNC:
        return "VIS_FTYPE_AUDIOSYNC";
    case VIS_MAX_FTYPE:
        return "VIS_MAX_FTYPE";
    default:
        return "Invalid Frame Type";
    }
}

static flist_node* flist_node_new(void) {
    flist_node* fn = DBMALLOC(sizeof(struct flist_node));
    fn->type = VIS_FTYPE_EMIT;
    return fn;
}

static void flist_node_free(flist_node* fn) {
    if (fn != NULL) {
        flist_node_free(fn->next);
        if (fn->type == VIS_FTYPE_EMIT) {
            emit_free(fn->data.frame);
        } else if (fn->type == VIS_FTYPE_CMD) {
            DZFREE(fn->data.frame);
        } else if (fn->type == VIS_FTYPE_BGCOLOR) {
        } else if (fn->type == VIS_FTYPE_MUTATE) {
            DZFREE(fn->data.method);
        } else if (fn->type == VIS_FTYPE_SCRIPTCB) {
            script_callback_free(fn->data.scriptcb);
        }
        DZFREE(fn);
    }
}

flist_t flist_new(void) {
    int i = 0;
    flist_t fl = DBMALLOC(sizeof(struct flist));
    while (i < VIS_NFRAMES) {
        fl->frames[i] = NULL;
        ++i;
    }
    fl->curr_frame = 0;
    return fl;
}

void flist_free(flist_t fl) {
    if (!fl) return;
    int i = 0;
    while (i < VIS_NFRAMES) {
        flist_node_free(fl->frames[i]);
        ++i;
    }
    DZFREE(fl);
}

void flist_insert(flist_t fl, fnum_t when, flist_node* fn) {
    VIS_ASSERT(fl);
    fl->total_frames += 1;
    if (fl->frames[when] == NULL) {
        fl->frames[when] = fn;
    } else {
        flist_node* curr = fl->frames[when];
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = fn;
    }
}

void flist_insert_emit(flist_t fl, fnum_t when, emit_desc* what) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_EMIT;
    fn->data.frame = what;
    flist_insert(fl, when, fn);
}

void flist_insert_exit(flist_t fl, fnum_t when) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_EXIT;
    flist_insert(fl, when, fn);
}

void flist_insert_play(flist_t fl, fnum_t when) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_PLAY;
    flist_insert(fl, when, fn);
}

void flist_insert_pause(flist_t fl, fnum_t when) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_PAUSE;
    flist_insert(fl, when, fn);
}

void flist_insert_volume(flist_t fl, fnum_t when, float volume) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_VOLUME;
    fn->data.volume = volume;
    flist_insert(fl, when, fn);
}

void flist_insert_audioseek(flist_t fl, fnum_t when, msec_t msec) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_AUDIOSEEK;
    fn->data.audioseek = msec;
    flist_insert(fl, when, fn);
}

void flist_insert_cmd(flist_t fl, fnum_t when, const char* what) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_CMD;
    fn->data.cmd = dupstr(what);
    flist_insert(fl, when, fn);
}

void flist_insert_bgcolor(flist_t fl, fnum_t when, float color[3]) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_BGCOLOR;
    fn->data.color[0] = color[0];
    fn->data.color[1] = color[1];
    fn->data.color[2] = color[2];
    flist_insert(fl, when, fn);
}

void flist_insert_mutate(flist_t fl, fnum_t when, mutate_method* method) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_MUTATE;
    fn->data.method = method;
    flist_insert(fl, when, fn);
}

void flist_insert_scriptcb(flist_t fl, fnum_t when, script_cb* func) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_SCRIPTCB;
    fn->data.scriptcb = func;
    flist_insert(fl, when, fn);
}

void flist_insert_seekframe(flist_t fl, fnum_t when, fnum_t where) {
    flist_node* fn = NULL;
    if (!fl || when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_FRAMESEEK;
    fn->data.frameseek = where;
    flist_insert(fl, when, fn);
}

void flist_insert_delay(flist_t fl, fnum_t when, fnum_t length) {
    flist_node* fn = NULL;
    if (!fl || when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_DELAY;
    fn->data.delay = length;
    flist_insert(fl, when, fn);
}

void flist_insert_audiosync(flist_t fl, fnum_t when, fnum_t length) {
    flist_node* fn = NULL;
    if (!fl || when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_AUDIOSYNC;
    fn->data.delay = length;
    flist_insert(fl, when, fn);
}

void flist_clear(flist_t fl) {
    int i = 0;
    if (!fl) return;
    while (i < VIS_NFRAMES) {
        flist_node_free(fl->frames[i]);
        fl->frames[i] = NULL;
        ++i;
    }
}

void flist_restart(flist_t fl) {
    if (!fl) return;
    fl->curr_frame = 0;
}

void flist_goto_frame(flist_t fl, fnum_t fn) {
    if (!fl) return;
    fl->curr_frame = fn;
}

BOOL flist_at_end(flist_t fl) {
    VIS_ASSERT(fl);
    return fl->curr_frame >= fl->total_frames;
}

flist_node* flist_tick(flist_t fl) {
    if (!fl || fl->curr_frame >= VIS_NFRAMES) return NULL;
#if DEBUG >= DEBUG_TRACE
    fprintf(stderr, "Current frame: %d\r", fl->curr_frame);
#endif
    return fl->frames[fl->curr_frame++];
}

flist_node* flist_node_next(flist_node* n) {
    return n ? n->next : NULL;
}
