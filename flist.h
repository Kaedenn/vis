
#ifndef VIS_FLIST_HEADER_INCLUDED_
#define VIS_FLIST_HEADER_INCLUDED_ 1

/* The frame list consists of a static array of list nodes. Each node
 * corresponds to a single frame of execution and can contain a pointer
 * to a subsequent node to invoke on the same frame.
 *
 * It is therefore an array of (often singlar) linked lists.
 */

#include "emit.h"
#include "mutator.h"
#include "script.h"
#include "types.h"

typedef struct flist_node {
    ftype_id type; /* types.h, identifies which field to use below */
    union {
        emit_desc* frame;      /* EMIT: emit frame */
        const char* cmd;       /* CMD: command frame */
        float color[3];        /* BGCOLOR: bgcolor frame */
        mutate_method* method; /* MUTATE: mutate frame */
        script_cb* scriptcb;   /* SCRIPTCB: lua invoke frame */
        fnum frameseek;        /* FRAMESEEK: go-to-frame-num frame */
        fnum delay;            /* DELAY|AUDIOSYNC: number of frames to wait */
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
void flist_insert_delay(flist* fl, fnum when, fnum length);
void flist_insert_audiosync(flist* fl, fnum when, fnum length);

#endif
