
#ifndef VIS_FLIST_HEADER_INCLUDED_
#define VIS_FLIST_HEADER_INCLUDED_ 1

#include "emit.h"
#include "mutator.h"
#include "script.h"

/* Frame List: an array of linked lists!
    This structure has an array of VIS_NFRAMES pointers to flist_nodes.
    
    In the current setup, that's ~35kB for one array.
*/

typedef struct flist_node {
    ftype_id type; /* types.h, identifies which field to use below */
    union {
        emit_t frame; /* VIS_FTYPE_EMIT: emit frame */
        const char* cmd; /* VIS_FTYPE_CMD: command frame */
        float color[3]; /* VIS_FTYPE_BGCOLOR: bgcolor frame */
        mutate_method_t method; /* VIS_FTYPE_MUTATE: mutate frame */
        script_cb_t scriptcb; /* VIS_FTYPE_SCRIPTCB: lua invoke frame */
        fnum_t frameseek; /* VIS_FTYPE_FRAMESEEK: go-to-frame-num frame */
    } data;
    struct flist_node* next;
} *flist_node_t;

typedef struct flist {
    flist_node_t frames[VIS_NFRAMES]; /* with apologies to the hardware */
    fnum_t total_frames;
    fnum_t curr_frame;
} *flist_t;

flist_t flist_new(void);
void flist_free(flist_t fl);
void flist_clear(flist_t fl);
void flist_restart(flist_t fl);
void flist_goto_frame(flist_t fl, fnum_t where);
BOOL flist_at_end(flist_t fl);

flist_node_t flist_tick(flist_t fl);
flist_node_t flist_node_next(flist_node_t n);

void flist_insert(flist_t fl, fnum_t when, flist_node_t fn);
void flist_insert_emit(flist_t fl, fnum_t when, emit_t what);
void flist_insert_exit(flist_t fl, fnum_t when);
void flist_insert_play(flist_t fl, fnum_t when);
void flist_insert_cmd(flist_t fl, fnum_t when, const char* what);
void flist_insert_bgcolor(flist_t fl, fnum_t when, float color[3]);
void flist_insert_mutate(flist_t fl, fnum_t when, mutate_method_t method);
void flist_insert_scriptcb(flist_t fl, fnum_t when, script_cb_t func);
void flist_insert_seekframe(flist_t fl, fnum_t when, fnum_t where);

#endif

