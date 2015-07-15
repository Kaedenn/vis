
#include "flist.h"
#include "helper.h"
#include "script.h"

static flist_node* flist_node_new(void) {
    flist_node* fn = DBMALLOC(sizeof(struct flist_node));
    if (!fn) return NULL;
    fn->type = VIS_FTYPE_EMIT;
    return fn;
}

static void flist_node_free(flist_node* fn) {
    if (fn != NULL) {
        flist_node_free(fn->next);
        if (fn->type == VIS_FTYPE_EMIT) {
            emit_free(fn->data.frame);
        } else if (fn->type == VIS_FTYPE_CMD) {
            DBFREE(fn->data.frame);
        } else if (fn->type == VIS_FTYPE_BGCOLOR) {
        } else if (fn->type == VIS_FTYPE_MUTATE) {
            DBFREE(fn->data.method);
        } else if (fn->type == VIS_FTYPE_SCRIPTCB) {
            script_callback_free(fn->data.scriptcb);
        }
        DBFREE(fn);
    }
}

flist* flist_new(void) {
    int i = 0;
    flist* fl = DBMALLOC(sizeof(struct flist));
    if (!fl) return NULL;
    while (i < VIS_NFRAMES) {
        fl->frames[i] = NULL;
        ++i;
    }
    fl->curr_frame = 0;
    return fl;
}

void flist_free(flist* fl) {
    int i = 0;
    while (i < VIS_NFRAMES) {
        flist_node_free(fl->frames[i]);
        ++i;
    }
    DBFREE(fl);
}

void flist_insert(flist* fl, fnum when, flist_node* fn) {
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

void flist_insert_emit(flist* fl, fnum when, emit_desc what) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_EMIT;
    fn->data.frame = what;
    flist_insert(fl, when, fn);
}

void flist_insert_exit(flist* fl, fnum when) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_EXIT;
    flist_insert(fl, when, fn);
}

void flist_insert_play(flist* fl, fnum when) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_PLAY;
    flist_insert(fl, when, fn);
}

void flist_insert_cmd(flist* fl, fnum when, const char* what) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_CMD;
    fn->data.cmd = dupstr(what);
    flist_insert(fl, when, fn);
}

void flist_insert_bgcolor(flist* fl, fnum when, float color[3]) {
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

void flist_insert_mutate(flist* fl, fnum when, mutate_method* method) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_MUTATE;
    fn->data.method = method;
    flist_insert(fl, when, fn);
}

void flist_insert_scriptcb(flist* fl, fnum when, script_cb* func) {
    flist_node* fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_SCRIPTCB;
    fn->data.scriptcb = func;
    flist_insert(fl, when, fn);
}

void flist_insert_seekframe(flist* fl, fnum when, fnum where) {
    flist_node* fn = NULL;
    if (!fl || when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_FRAMESEEK;
    fn->data.frameseek = where;
    flist_insert(fl, when, fn);
}

void flist_clear(flist* fl) {
    int i = 0;
    if (!fl) return;
    while (i < VIS_NFRAMES) {
        flist_node_free(fl->frames[i]);
        fl->frames[i] = NULL;
        ++i;
    }
}

void flist_restart(flist* fl) {
    if (!fl) return;
    fl->curr_frame = 0;
}

void flist_goto_frame(flist* fl, fnum fn) {
    if (!fl) return;
    fl->curr_frame = fn;
}

BOOL flist_at_end(flist* fl) {
    VIS_ASSERT(fl);
    return fl->curr_frame >= fl->total_frames;
}

flist_node* flist_tick(flist* fl) {
    if (!fl || fl->curr_frame >= VIS_NFRAMES) return NULL;
#if DEBUG >= DEBUG_TRACE
    fprintf(stderr, "Current frame: %d\r", fl->curr_frame);
#endif
    return fl->frames[fl->curr_frame++];
}

flist_node* flist_node_next(flist_node* n) {
    return n ? n->next : NULL;
}

