
#ifndef VIS_SCRIPT_HEADER_INCLUDED_
#define VIS_SCRIPT_HEADER_INCLUDED_ 1

#include "drawer.h"

struct flist;

typedef struct script_cb {
    struct script* owner;
    char* fn_name;
    char* fn_code;
} *script_cb_t;

typedef struct script* script_t;

typedef unsigned int script_cfg_t;
static const script_cfg_t SCRIPT_ALLOW_ALL = 0; /* allow all features */
static const script_cfg_t SCRIPT_NO_CB = 1; /* disable callbacks */

char* genlua_emit(emit_t emit, fnum_t when);
const char* genlua_force(force_id force);
const char* genlua_limit(limit_id limit);
const char* genlua_blender(blend_id blender);

script_t script_new(script_cfg_t cfg);
void script_free(script_t s);
void script_callback_free(script_cb_t cb);

struct flist* script_run(script_t script, const char* filename);
void script_run_string(script_t script, const char* torun);
void call_script(script_t state, script_cb_t func, void* args);
void script_set_drawer(script_t script, drawer_t drawer);

void script_mousemove(script_t script, int x, int y);
void script_mousedown(script_t script, int x, int y);
void script_mouseup(script_t script, int x, int y);
void script_keydown(script_t script, const char* keyname);
void script_keyup(script_t script, const char* keyname);
void script_quit(script_t script);

#endif

