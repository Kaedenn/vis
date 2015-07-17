
#ifndef VIS_SCRIPT_HEADER_INCLUDED_
#define VIS_SCRIPT_HEADER_INCLUDED_ 1

#include "drawer.h"
#include <stdint.h>

/* FIXME: circular dependency between flist.h and script.h */
struct flist;

typedef struct script_cb {
    struct script* owner;
    char* fn_name;
    char* fn_code;
} script_cb;

typedef struct script* script_t;

enum script_debug_id {
    SCRIPT_DEBUG_PARTICLES_EMITTED,
    SCRIPT_DEBUG_TIME_NOW,
    SCRIPT_DEBUG_FRAMES_EMITTED,
    SCRIPT_DEBUG_NUM_MUTATES,
    SCRIPT_DEBUG_PARTICLES_MUTATED,
    SCRIPT_DEBUG_PARTICLE_TAGS_MODIFIED
};

typedef struct script_debug {
    uint64_t particles_emitted;
    uint64_t time_now;
    uint64_t frames_emitted;
    uint64_t num_mutates;
    uint64_t particles_mutated;
    uint64_t particle_tags_modified;
} script_debug;

typedef unsigned int script_cfg_mask;
static const script_cfg_mask SCRIPT_ALLOW_ALL = 0; /* allow all features */
static const script_cfg_mask SCRIPT_NO_CB = 1; /* disable Vis.callback() */
static const script_cfg_mask SCRIPT_NO_EXIT = 2; /* disable Vis.exit() */

script_t script_new(script_cfg_mask cfg);
void script_free(script_t s);
void script_callback_free(script_cb* cb);

/* number of errors returned to the script API, 0 for success */
int script_get_status(script_t s);

void script_set_debug(script_t s, enum script_debug_id what, uint64_t n);
void script_get_debug(script_t s, /*out*/ script_debug* dbg);

struct flist* script_run(script_t script, const char* filename);
void script_run_string(script_t script, const char* torun);
void script_run_cb(script_t state, script_cb* func, void* args);
void script_set_drawer(script_t script, drawer_t drawer);

void script_mousemove(script_t script, int x, int y);
void script_mousedown(script_t script, int x, int y, int button);
void script_mouseup(script_t script, int x, int y, int button);
void script_keydown(script_t script, const char* keyname, BOOL shift);
void script_keyup(script_t script, const char* keyname, BOOL shift);
void script_on_quit(script_t script);

#endif

