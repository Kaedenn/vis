
#ifndef VIS_FLIST_HEADER_INCLUDED_
#define VIS_FLIST_HEADER_INCLUDED_ 1

#include "emit.h"
#include "mutator.h"
#include "script.h"
#include "types.h"

typedef struct flist_node {
    ftype_id type; /* types.h, identifies which field to use below */
    union {
        emit_desc* frame;      /* VIS_FTYPE_EMIT: emit frame */
        const char* cmd;       /* VIS_FTYPE_CMD: command frame */
        float color[3];        /* VIS_FTYPE_BGCOLOR: bgcolor frame */
        mutate_method* method; /* VIS_FTYPE_MUTATE: mutate frame */
        script_cb* scriptcb;   /* VIS_FTYPE_SCRIPTCB: lua invoke frame */
        fnum frameseek;        /* VIS_FTYPE_FRAMESEEK: go-to-frame-num frame */
    } data;
    struct flist_node* next;
} flist_node;

typedef struct flist {
    fnum total_frames;
    fnum curr_frame;
    flist_node* frames[VIS_NFRAMES]; /* with apologies to the hardware */
} flist;

flist* flist_new(void);
void flist_free(flist* fl);
void flist_clear(flist* fl);
void flist_restart(flist* fl);
void flist_goto_frame(flist* fl, fnum where);
BOOL flist_at_end(flist* fl);

flist_node* flist_tick(flist* fl);
flist_node* flist_node_next(flist_node* n);

void flist_insert(flist* fl, fnum when, flist_node* fn);
void flist_insert_emit(flist* fl, fnum when, emit_desc* what);
void flist_insert_exit(flist* fl, fnum when);
void flist_insert_play(flist* fl, fnum when);
void flist_insert_cmd(flist* fl, fnum when, const char* what);
void flist_insert_bgcolor(flist* fl, fnum when, float color[3]);
void flist_insert_mutate(flist* fl, fnum when, mutate_method* method);
void flist_insert_scriptcb(flist* fl, fnum when, script_cb* func);
void flist_insert_seekframe(flist* fl, fnum when, fnum where);

#endif
