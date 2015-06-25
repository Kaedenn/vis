
#ifndef VIS_FLIST_HEADER_INCLUDED_
#define VIS_FLIST_HEADER_INCLUDED_ 1

#include "frame.h"
#include "script.h"

/* Frame List: an array of linked lists!
    This structure has an array of VIS_NFRAMES pointers to flist_nodes.
    
    In the current setup, that's ~35kB for one array.
*/

typedef struct flist_node {
    frame_type_t type; /* defines.h: VIS_FTYPE_* */
    union {
        emit_t frame; /* emit frame */
        const char* cmd; /* command frame */
        float color[3]; /* bgcolor frame */
        mutate_method_t method; /* mutate frame */
        script_cb_t scriptcb; /* lua invoke frame */
        fnum_t frameseek; /* go-to-frame-num frame */
    } data;
    struct flist_node* next;
} *flist_node_t;

typedef struct flist {
    flist_node_t frames[VIS_NFRAMES]; /* with apologies to the hardware */
    fnum_t curr_frame;
} *flist_t;

flist_t flist_new(void);
void flist_free(flist_t fl);
void flist_insert(flist_t fl, fnum_t when, flist_node_t fn);
void flist_insert_emit(flist_t fl, fnum_t when, emit_t what);
void flist_insert_cmd(flist_t fl, fnum_t when, const char* what);
void flist_insert_bgcolor(flist_t fl, fnum_t when, float color[3]);
void flist_insert_mutate(flist_t fl, fnum_t when, mutate_method_t method);
void flist_insert_scriptcb(flist_t fl, fnum_t when, script_cb_t func);
void flist_insert_seekframe(flist_t fl, fnum_t when, fnum_t where);
void flist_clear(flist_t fl);
void flist_restart(flist_t fl);
void flist_goto_frame(flist_t fl, fnum_t where);

flist_node_t flist_tick(flist_t fl);
flist_node_t flist_node_next(flist_node_t n);

#endif

