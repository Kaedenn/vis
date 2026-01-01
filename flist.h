
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
        unsigned audioseek;    /* AUDIOSEEK: audio seek milliseconds */
        fnum frameseek;        /* FRAMESEEK: go-to-frame-num frame */
        float volume;          /* VOLUME: set volume frame */
        fnum delay;            /* DELAY|AUDIOSYNC: number of frames to wait */
    } data;
    struct flist_node* next;
} flist_node;

typedef struct flist {
    fnum total_frames;
    fnum curr_frame;
    flist_node* frames[VIS_NFRAMES]; /* with apologies to the hardware */
} flist;

typedef flist* flist_t;

const char* ftype_to_string(ftype_id ftype);

flist_t flist_new(void);
void flist_free(flist_t fl);
void flist_clear(flist_t fl);
void flist_restart(flist_t fl);
void flist_goto_frame(flist_t fl, fnum where);
BOOL flist_at_end(flist_t fl);

flist_node* flist_tick(flist_t fl);
flist_node* flist_node_next(flist_node* n);

void flist_insert(flist_t fl, fnum when, flist_node* fn);
void flist_insert_emit(flist_t fl, fnum when, emit_desc* what);
void flist_insert_exit(flist_t fl, fnum when);
void flist_insert_play(flist_t fl, fnum when);
void flist_insert_pause(flist_t fl, fnum when);
void flist_insert_volume(flist_t fl, fnum when, float volume);
void flist_insert_audioseek(flist_t fl, fnum when, unsigned msec);
void flist_insert_cmd(flist_t fl, fnum when, const char* what);
void flist_insert_bgcolor(flist_t fl, fnum when, float color[3]);
void flist_insert_mutate(flist_t fl, fnum when, mutate_method* method);
void flist_insert_scriptcb(flist_t fl, fnum when, script_cb* func);
void flist_insert_seekframe(flist_t fl, fnum when, fnum where);
void flist_insert_delay(flist_t fl, fnum when, fnum length);
void flist_insert_audiosync(flist_t fl, fnum when, fnum length);

#endif
