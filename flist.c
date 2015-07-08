
#include "flist.h"
#include "helper.h"
#include "script.h"

static flist_node_t flist_node_new(void) {
    flist_node_t fn = DBMALLOC(sizeof(struct flist_node));
    if (!fn) return NULL;
    fn->type = VIS_FTYPE_EMIT;
    return fn;
}

static void flist_node_free(flist_node_t fn) {
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

flist_t flist_new(void) {
    int i = 0;
    flist_t fl = DBMALLOC(sizeof(struct flist));
    if (!fl) return NULL;
    while (i < VIS_NFRAMES) {
        fl->frames[i] = NULL;
        ++i;
    }
    fl->curr_frame = 0;
    return fl;
}

void flist_free(flist_t fl) {
    int i = 0;
    while (i < VIS_NFRAMES) {
        flist_node_free(fl->frames[i]);
        ++i;
    }
    DBFREE(fl);
}

void flist_insert(flist_t fl, fnum_t when, flist_node_t fn) {
    if (fl->frames[when] == NULL) {
        fl->frames[when] = fn;
    } else {
        flist_node_t curr = fl->frames[when];
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = fn;
    }
}

void flist_insert_emit(flist_t fl, fnum_t when, emit_t what) {
    flist_node_t fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_EMIT;
    fn->data.frame = what;
    flist_insert(fl, when, fn);
}

void flist_insert_exit(flist_t fl, fnum_t when) {
    flist_node_t fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_EXIT;
    flist_insert(fl, when, fn);
}

void flist_insert_cmd(flist_t fl, fnum_t when, const char* what) {
    flist_node_t fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_CMD;
    fn->data.cmd = dupstr(what);
    flist_insert(fl, when, fn);
}

void flist_insert_bgcolor(flist_t fl, fnum_t when, float color[3]) {
    flist_node_t fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_BGCOLOR;
    fn->data.color[0] = color[0];
    fn->data.color[1] = color[1];
    fn->data.color[2] = color[2];
    flist_insert(fl, when, fn);
}

void flist_insert_mutate(flist_t fl, fnum_t when, mutate_method_t method) {
    flist_node_t fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_MUTATE;
    fn->data.method = method;
    flist_insert(fl, when, fn);
}

void flist_insert_scriptcb(flist_t fl, fnum_t when, script_cb_t func) {
    flist_node_t fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_SCRIPTCB;
    fn->data.scriptcb = func;
    flist_insert(fl, when, fn);
}

void flist_insert_seekframe(flist_t fl, fnum_t when, fnum_t where) {
    flist_node_t fn = NULL;
    if (!fl || when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_FRAMESEEK;
    fn->data.frameseek = where;
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

flist_node_t flist_tick(flist_t fl) {
    if (!fl || fl->curr_frame >= VIS_NFRAMES) return NULL;
#if DEBUG >= DEBUG_TRACE
    fprintf(stderr, "Current frame: %d\r", fl->curr_frame);
#endif
    return fl->frames[fl->curr_frame++];
}

flist_node_t flist_node_next(flist_node_t n) {
    return n ? n->next : NULL;
}

