
#include "flist.h"
#include "helper.h"

static flist_node_t flist_node_new(void) {
    flist_node_t fn = chmalloc(sizeof(struct flist_node));
    if (!fn) return NULL;
    fn->type = VIS_FTYPE_EMIT;
    fn->data.frame = NULL;
    fn->next = NULL;
    return fn;
}

static void flist_node_free(flist_node_t fn) {
    if (fn != NULL) {
        flist_node_free(fn->next);
        if (fn->type == VIS_FTYPE_EMIT) {
            free_frame(fn->data.frame);
        } else if (fn->type == VIS_FTYPE_CMD) {
            free(fn->data.frame);
        }
        free(fn);
    }
}

flist_t flist_new(void) {
    int i = 0;
    flist_t fl = chmalloc(sizeof(struct flist));
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
    free(fl);
}

void flist_insert_emit(flist_t fl, fnum_t when, frame_t what) {
    flist_node_t fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_EMIT;
    fn->data.frame = what;
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

void flist_insert_cmd(flist_t fl, fnum_t when, const char* what) {
    flist_node_t fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_CMD;
    fn->data.cmd = dupstr(what);
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

void flist_insert_bgcolor(flist_t fl, fnum_t when, float color[3]) {
    flist_node_t fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_BGCOLOR;
    fn->data.color[0] = color[0];
    fn->data.color[1] = color[1];
    fn->data.color[2] = color[2];
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

void flist_insert_mutate(flist_t fl, fnum_t when, mutate_method_t method) {
    flist_node_t fn = NULL;
    if (!fl) return;
    if (when >= VIS_NFRAMES) return;
    fn = flist_node_new();
    fn->type = VIS_FTYPE_MUTATE;
    fn->data.method = method;
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

flist_node_t flist_tick(flist_t fl) {
    if (!fl || fl->curr_frame == VIS_NFRAMES) return NULL;
    return fl->frames[fl->curr_frame++];
}

