
#include "script.h"
#include "audio.h"
#include "emitter.h"
#include "flist.h"
#include "genlua.h"
#include "helper.h"
#include "kstring.h"
#include "mutator.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

/* Initialization script */
static const char* const LUA_INIT_SCRIPT =
    /* Adjust default Lua search path */
    "package = require('package')\n"
    "package.path = '; ;./?.lua;./lua/?.lua;./test/?.lua'"
    /* Add default modules */
    "VisUtil = require(\"visutil\")\n"
    "Emit = require(\"emit\")\n"
    /* Create tables for user input handling (used by script API) */
    "Vis._on_mousedowns = {}\n"
    "Vis._on_mouseups = {}\n"
    "Vis._on_mousemoves = {}\n"
    "Vis._on_mousescrolls = {}\n"
    "Vis._on_keydowns = {}\n"
    "Vis._on_keyups = {}\n"
    "Vis._on_quits = {}\n"
    /* Create functions for interacting with said tables */
    "Vis._do_on_event = function(tab, ...)\n"
    "   for i,f in pairs(tab) do f(...) end\n"
    "end\n"
    "Vis.on_mousedown = function(f)\n"
    "   table.insert(Vis._on_mousedowns, f)\n"
    "end\n"
    "Vis.on_mouseup = function(f)\n"
    "   table.insert(Vis._on_mouseups, f)\n"
    "end\n"
    "Vis.on_mousemove = function(f)\n"
    "   table.insert(Vis._on_mousemoves, f)\n"
    "end\n"
    "Vis.on_mousescroll = function(f)\n"
    "   table.insert(Vis._on_mousescrolls, f)\n"
    "end\n"
    "Vis.on_keydown = function(f)\n"
    "   table.insert(Vis._on_keydowns, f)\n"
    "end\n"
    "Vis.on_keyup = function(f)\n"
    "   table.insert(Vis._on_keyups, f)\n"
    "end\n"
    "Vis.on_quit = function(f)\n"
    "   table.insert(Vis._on_quits, f)\n"
    "end\n";

/* Helper functions not exposed to Lua */
static int initialize_vis_lib(lua_State* L);
static void prepare_stack(script_t s, klist args);
static void cleanup_stack(script_t s);
static script_t util_checkscript(lua_State* L, int pos);
static const char* util_get_error(lua_State* L);
static emit_desc* lua_args_to_emit_desc(lua_State* L, int arg, fnum* when);
static void push_constant_num(lua_State* L, const char* k, double v, int idx);
static void push_constant_int(lua_State* L, const char* k, int v, int idx);
static int do_mouse_event(lua_State* L, const char* fn, int x, int y, int b);
static int do_key_event(lua_State* L, const char* fn, const char* k, BOOL shft);
static int get_configured_fps(lua_State* L);
static int do_frames2msec(lua_State* L, fnum frame);
static fnum do_msec2frames(lua_State* L, long msec);
static kstr lua_inspect_value(lua_State* L, int arg);

/* Functions exposed to Lua */
static int viscmd_debug_fn(lua_State* L);
static int viscmd_command_fn(lua_State* L);
static int viscmd_exit_fn(lua_State* L);
static int viscmd_emit_fn(lua_State* L);
static int viscmd_audio_fn(lua_State* L);
static int viscmd_play_fn(lua_State* L);
static int viscmd_pause_fn(lua_State* L);
static int viscmd_volume_fn(lua_State* L);
static int viscmd_seek_fn(lua_State* L);
static int viscmd_seekms_fn(lua_State* L);
static int viscmd_seekframe_fn(lua_State* L);
static int viscmd_gotoframe_fn(lua_State* L);
static int viscmd_audiosync_fn(lua_State* L);
static int viscmd_delay_fn(lua_State* L);
static int viscmd_bgcolor_fn(lua_State* L);
static int viscmd_mutate_fn(lua_State* L);
static int viscmd_callback_fn(lua_State* L);
static int viscmd_fps_fn(lua_State* L);
static int viscmd_settrace_fn(lua_State* L);
static int viscmd_emitnow_fn(lua_State* L);
static int viscmd_f2ms_fn(lua_State* L);
static int viscmd_ms2f_fn(lua_State* L);
static int viscmd_get_debug_fn(lua_State* L);

#define SCRIPT_RUN_STRINGVF(s, fmt, ...) do { \
    kstr script_run_kstring_temp = kstring_newfromvf((fmt), __VA_ARGS__); \
    script_run_string((s), kstring_content(script_run_kstring_temp)); \
    kstring_free(script_run_kstring_temp); \
} while(0)

struct script {
    script_debug* dbg;
    lua_State* L;
    int debugidx;
    klist args;
    flist_t fl;
    drawer_t drawer;
    int errors;
};

/* start of public API */
script_t script_new(script_cfg_mask cfg, const clargs* args) {
    script_t s = DBMALLOC(sizeof(struct script));
    s->dbg = DBMALLOC(sizeof(struct script_debug));
    s->fl = flist_new();
    s->L = luaL_newstate();
    luaL_openlibs(s->L);
    luaL_requiref(s->L, "Vis", initialize_vis_lib, 0);
    lua_pop(s->L, 1); /* requiref("Vis") */

    script_run_string(s, LUA_INIT_SCRIPT);
    VIS_ASSERT(s->errors == 0);
    if (fexists(LUA_STARTUP_FILE)) {
        DBPRINTF("Executing startup file: %s", LUA_STARTUP_FILE);
        script_run(s, LUA_STARTUP_FILE);
    }

    lua_getglobal(s->L, "Vis");

    /* Assign WIDTH, HEIGHT constants */
    push_constant_int(s->L, "WIDTH", (int)args->wsize[0], -1);
    push_constant_int(s->L, "HEIGHT", (int)args->wsize[1], -1);

    /* Create and assign Vis.flist */
    flist_t* flbox = lua_newuserdata(s->L, sizeof(flist_t));
    luaL_newmetatable(s->L, "flist**");
    lua_pop(s->L, 1);
    luaL_setmetatable(s->L, "flist**");
    *flbox = s->fl;
    lua_setfield(s->L, -2, "flist");

    /* Create and assign Vis.script (if enabled) */
    if ((cfg & SCRIPT_NO_CB) == 0) {
        script_t* sbox = lua_newuserdata(s->L, sizeof(script_t));
        luaL_newmetatable(s->L, "script_t*");
        lua_pop(s->L, 1);
        luaL_setmetatable(s->L, "script_t*");
        *sbox = s;
        lua_setfield(s->L, -2, "script");
    }

    if ((cfg & SCRIPT_NO_EXIT) != 0) {
        script_run_string(s, "Vis.exit = function() end");
        VIS_ASSERT(s->errors == 0);
    }

    lua_pop(s->L, 1); /* getglobal("Vis") */
    VIS_ASSERT(lua_gettop(s->L) == 0);

    return s;
}

void script_free(script_t s) {
    if (!s)
        return;
    lua_close(s->L);
    if (s->args)
        klist_free(s->args);
    DZFREE(s->dbg);
    DZFREE(s);
}

void script_set_args(script_t s, klist args) {
    s->args = args;
}

flist_t script_run(script_t s, const char* filename) {
    /* s->fl already bound to script in script_new */
    /* DBPRINTF("Running %s to build %p", filename, s->fl); */
    prepare_stack(s, s->args);
    if (luaL_loadfile(s->L, filename) != LUA_OK) {
        s->errors += 1;
        EPRINTF("Error in script %s: %s", filename, util_get_error(s->L));
    } else if (lua_pcall(s->L, 0, LUA_MULTRET, s->debugidx) != LUA_OK) {
        s->errors += 1;
        DBPRINTF("%d items left on the stack", lua_gettop(s->L));
        EPRINTF("Error in script: %s: %s", filename, util_get_error(s->L));
    }
    cleanup_stack(s);
    return s->fl;
}

void script_run_string(script_t s, const char* torun) {
    prepare_stack(s, s->args);
    char* esc = escape_string(torun);
    if (luaL_loadstring(s->L, torun) != LUA_OK) {
        s->errors += 1;
        EPRINTF("Syntax error in script \"%s\": %s", esc, util_get_error(s->L));
    } else if (lua_pcall(s->L, 0, LUA_MULTRET, s->debugidx) != LUA_OK) {
        s->errors += 1;
        EPRINTF("Call error in script \"%s\": %s", esc, util_get_error(s->L));
    }
    DZFREE(esc);
    cleanup_stack(s);
}

void script_run_cb(script_t s, script_cb* cb, UNUSED_PARAM(void* args)) {
    script_run_string(s, cb->fn_code);
}

void script_set_drawer(script_t s, drawer_t drawer) {
    s->drawer = drawer;
    lua_getglobal(s->L, "Vis");
    push_constant_int(s->L, "FPS_LIMIT", drawer_get_configured_fps(drawer), -1);
    lua_pop(s->L, 1); /* getglobal("Vis") */
}

void script_callback_free(script_cb* cb) {
    DZFREE(cb->fn_name);
    DZFREE(cb->fn_code);
    DZFREE(cb);
}

int script_get_status(script_t s) {
    return s->errors;
}

void script_clear_status(script_t s) {
    s->errors = 0;
}

void script_set_debug(script_t s, enum script_debug_id what, uint64_t n) {
    switch (what) {
    case SCRIPT_DEBUG_PARTICLES_EMITTED:
        s->dbg->particles_emitted = n;
        break;
    case SCRIPT_DEBUG_TIME_NOW:
        s->dbg->time_now = n;
        break;
    case SCRIPT_DEBUG_FRAMES_EMITTED:
        s->dbg->frames_emitted = n;
        break;
    case SCRIPT_DEBUG_NUM_MUTATES:
        s->dbg->num_mutates = n;
        break;
    case SCRIPT_DEBUG_PARTICLES_MUTATED:
        s->dbg->particles_mutated = n;
        break;
    case SCRIPT_DEBUG_PARTICLE_TAGS_MODIFIED:
        s->dbg->particle_tags_modified = n;
        break;
    }
}

void script_get_debug(script_t s, script_debug* dbg) {
    *dbg = *s->dbg;
}

void script_mousescroll(script_t s, int xoffset, int yoffset) {
    s->errors += do_mouse_event(s->L, "mousescroll", xoffset, yoffset, 0);
}

void script_mousemove(script_t s, int x, int y) {
    s->errors += do_mouse_event(s->L, "mousemove", x, y, 0);
}

void script_mousedown(script_t s, int x, int y, int button) {
    s->errors += do_mouse_event(s->L, "mousedown", x, y, button);
}

void script_mouseup(script_t s, int x, int y, int button) {
    s->errors += do_mouse_event(s->L, "mouseup", x, y, button);
}

void script_keydown(script_t s, const char* keyname, BOOL shift) {
    s->errors += do_key_event(s->L, "keydown", keyname, shift);
}

void script_keyup(script_t s, const char* keyname, BOOL shift) {
    s->errors += do_key_event(s->L, "keyup", keyname, shift);
}

void script_on_quit(script_t s) {
    script_run_string(s, "Vis._do_on_event(Vis._on_quits)");
}
/* end of public API */

/* begin of private API */
int initialize_vis_lib(lua_State* L) {
    static const struct luaL_Reg vis_lib[] = {
        {"debug", viscmd_debug_fn},
        {"command", viscmd_command_fn},
        {"exit", viscmd_exit_fn},
        {"emit", viscmd_emit_fn},
        {"audio", viscmd_audio_fn},
        {"play", viscmd_play_fn},
        {"pause", viscmd_pause_fn},
        {"volume", viscmd_volume_fn},
        {"seek", viscmd_seek_fn},
        {"seekms", viscmd_seekms_fn},
        {"seekframe", viscmd_seekframe_fn},
        {"gotoframe", viscmd_gotoframe_fn},
        {"audiosync", viscmd_audiosync_fn},
        {"delay", viscmd_delay_fn},
        {"bgcolor", viscmd_bgcolor_fn},
        {"mutate", viscmd_mutate_fn},
        {"callback", viscmd_callback_fn},
        {"fps", viscmd_fps_fn},
        {"settrace", viscmd_settrace_fn},
        {"frames2msec", viscmd_f2ms_fn},
        {"msec2frames", viscmd_ms2f_fn},
        {"emitnow", viscmd_emitnow_fn},
        {"get_debug", viscmd_get_debug_fn},
        {NULL, NULL}};

    luaL_newlib(L, vis_lib);

#define NEW_VIS_CONST_INT(name) push_constant_int(L, (#name), (VIS_##name), -1)
#define NEW_VIS_CONST_NUM(name) push_constant_num(L, (#name), (VIS_##name), -1)

    /* grant access to all of the enums and constants */
    NEW_VIS_CONST_INT(FPS_LIMIT);
    /* blenders */
    NEW_VIS_CONST_INT(DEFAULT_BLEND);
    NEW_VIS_CONST_INT(BLEND_NONE);
    NEW_VIS_CONST_INT(BLEND_LINEAR);
    NEW_VIS_CONST_INT(BLEND_PARABOLIC);
    NEW_VIS_CONST_INT(BLEND_QUADRATIC);
    NEW_VIS_CONST_INT(BLEND_SINE);
    NEW_VIS_CONST_INT(BLEND_NEGGAMMA);
    NEW_VIS_CONST_INT(BLEND_EASING);
    NEW_VIS_CONST_INT(NBLENDS);
    /* forces */
    NEW_VIS_CONST_INT(DEFAULT_FORCE);
    NEW_VIS_CONST_INT(FORCE_FRICTION);
    NEW_VIS_CONST_INT(FORCE_GRAVITY);
    NEW_VIS_CONST_INT(NFORCES);
    /* limits */
    NEW_VIS_CONST_INT(DEFAULT_LIMIT);
    NEW_VIS_CONST_INT(LIMIT_BOX);
    NEW_VIS_CONST_INT(LIMIT_SPRINGBOX);
    NEW_VIS_CONST_INT(NLIMITS);
    /* mutators */
    NEW_VIS_CONST_INT(MUTATE_PUSH);
    NEW_VIS_CONST_INT(MUTATE_PUSH_DX);
    NEW_VIS_CONST_INT(MUTATE_PUSH_DY);
    NEW_VIS_CONST_INT(MUTATE_SLOW);
    NEW_VIS_CONST_INT(MUTATE_SHRINK);
    NEW_VIS_CONST_INT(MUTATE_GROW);
    NEW_VIS_CONST_INT(MUTATE_AGE);
    NEW_VIS_CONST_INT(MUTATE_OPACITY);
    NEW_VIS_CONST_INT(MUTATE_SET_DX);
    NEW_VIS_CONST_INT(MUTATE_SET_DY);
    NEW_VIS_CONST_INT(MUTATE_SET_RADIUS);
    /* tag mutators */
    NEW_VIS_CONST_INT(MUTATE_TAG_SET);
    NEW_VIS_CONST_INT(MUTATE_TAG_INC);
    NEW_VIS_CONST_INT(MUTATE_TAG_DEC);
    NEW_VIS_CONST_INT(MUTATE_TAG_ADD);
    NEW_VIS_CONST_INT(MUTATE_TAG_SUB);
    NEW_VIS_CONST_INT(MUTATE_TAG_MUL);
    NEW_VIS_CONST_INT(MUTATE_TAG_DIV);
    /* conditional mutators */
    NEW_VIS_CONST_INT(MUTATE_PUSH_IF);
    NEW_VIS_CONST_INT(MUTATE_PUSH_DX_IF);
    NEW_VIS_CONST_INT(MUTATE_PUSH_DY_IF);
    NEW_VIS_CONST_INT(MUTATE_SLOW_IF);
    NEW_VIS_CONST_INT(MUTATE_SHRINK_IF);
    NEW_VIS_CONST_INT(MUTATE_GROW_IF);
    NEW_VIS_CONST_INT(MUTATE_AGE_IF);
    NEW_VIS_CONST_INT(MUTATE_OPACITY_IF);
    NEW_VIS_CONST_INT(MUTATE_SET_DX_IF);
    NEW_VIS_CONST_INT(MUTATE_SET_DY_IF);
    NEW_VIS_CONST_INT(MUTATE_SET_RADIUS_IF);
    /* total number of mutators */
    NEW_VIS_CONST_INT(NMUTATES);
    /* mutate conditions */
    NEW_VIS_CONST_INT(MUTATE_IF_TRUE);
    NEW_VIS_CONST_INT(MUTATE_IF_EQ);
    NEW_VIS_CONST_INT(MUTATE_IF_NE);
    NEW_VIS_CONST_INT(MUTATE_IF_LT);
    NEW_VIS_CONST_INT(MUTATE_IF_LE);
    NEW_VIS_CONST_INT(MUTATE_IF_GT);
    NEW_VIS_CONST_INT(MUTATE_IF_GE);
    NEW_VIS_CONST_INT(MUTATE_IF_EVEN);
    NEW_VIS_CONST_INT(MUTATE_IF_ODD);
    /* other constants */
    NEW_VIS_CONST_NUM(FORCE_FRICTION_COEFF);
    NEW_VIS_CONST_NUM(FORCE_GRAVITY_FACTOR);
    NEW_VIS_CONST_INT(NFRAMES);
    NEW_VIS_CONST_INT(AUDIO_FREQ);
    NEW_VIS_CONST_INT(AUDIO_SAMPLES);
    NEW_VIS_CONST_INT(AUDIO_CHANNELS);
#undef NEW_VIS_CONST_INT
#undef NEW_VIS_CONST_NUM
    /* helpful non-Vis constants */
    push_constant_int(L, "MOUSE_LMB", 1, -1);
    push_constant_int(L, "MOUSE_MMB", 2, -1);
    push_constant_int(L, "MOUSE_RMB", 3, -1);
    push_constant_int(L, "MOUSE_XMB", 4, -1);
    push_constant_int(L, "MOUSE_YMB", 5, -1);
    push_constant_num(L, "CONST_PUSH_STOP", 0.0, -1);
    push_constant_num(L, "CONST_AGE_BORN", 1.0, -1);
    push_constant_num(L, "CONST_AGE_DEAD", 0.0, -1);
    push_constant_int(L, "DEBUG", DEBUG, -1);
    push_constant_int(L, "DEBUG_NONE", DEBUG_NONE, -1);
    push_constant_int(L, "DEBUG_VERBOSE", DEBUG_VERBOSE, -1);
    push_constant_int(L, "DEBUG_DEBUG", DEBUG_DEBUG, -1);
    push_constant_int(L, "DEBUG_INFO", DEBUG_INFO, -1);
    push_constant_int(L, "DEBUG_TRACE", DEBUG_TRACE, -1);
    /* helpful non-numeric constants */
    lua_pushstring(L, "LUA_STARTUP_FILE");
    lua_pushstring(L, LUA_STARTUP_FILE);
    lua_settable(L, -3);

    return 1;
}

void prepare_stack(script_t s, klist args) {
    lua_getglobal(s->L, "debug");
    lua_getfield(s->L, -1, "traceback");
    s->debugidx = lua_gettop(s->L);
    lua_pushglobaltable(s->L);
    lua_pushstring(s->L, "Arguments");
    if (args != NULL) {
        lua_createtable(s->L, (int)klist_length(args), 0);
        for (size_t i = 0; i < klist_length(args); ++i) {
            lua_pushinteger(s->L, (int)i + 1);
            lua_pushstring(s->L, klist_getn(args, i));
            lua_settable(s->L, -3);
        }
    } else {
        lua_createtable(s->L, 0, 0);
    }
    lua_settable(s->L, -3);
    lua_pop(s->L, 1); /* pushglobaltable(s->L) */
    VIS_ASSERT(lua_gettop(s->L) == 2);
}

void cleanup_stack(script_t s) {
    lua_pop(s->L, lua_gettop(s->L));
}

void push_constant_num(lua_State* L, const char* k, double v, int idx) {
    lua_pushstring(L, k);
    lua_pushnumber(L, v);
    lua_settable(L, idx - 2);
}

void push_constant_int(lua_State* L, const char* k, int v, int idx) {
    lua_pushstring(L, k);
    lua_pushinteger(L, v);
    lua_settable(L, idx - 2);
}

script_t util_checkscript(lua_State* L, int arg) {
    if (!lua_isnil(L, arg)) {
        return *(script_t*)luaL_checkudata(L, arg++, "script_t*");
    }
    luaL_error(L, "Vis.script is nil; function may be disabled");
    return NULL;
}

drawer_t util_checkdrawer(lua_State* L, script_t s) {
    if (s->drawer) {
        return s->drawer;
    }
    luaL_error(L, "script has no drawer; function may be disabled");
    return NULL;
}

const char* util_get_error(lua_State* L) {
    static char error_buff[2048];
    strncpy(error_buff, luaL_checkstring(L, -1), 2048);
    lua_pop(L, 1);
    return error_buff;
}

int do_mouse_event(lua_State* L, const char* func, int x, int y, int button) {
    int nerror = 0;
    kstr s = kstring_newfromvf(
            "Vis._do_on_event(Vis._on_%ss, %d, %d, %d)", func, x, y, button);
    if (luaL_dostring(L, kstring_content(s)) != LUA_OK) {
        EPRINTF("Error in %s: %s", kstring_content(s), util_get_error(L));
        nerror = 1;
    }
    kstring_free(s);
    return nerror;
}

int do_key_event(lua_State* L, const char* func, const char* key, BOOL shift) {
    int nerror = 0;
    char* esc_key = escape_string(key);
    kstr s = kstring_newfromvf(
        "Vis._do_on_event(Vis._on_%ss, \"%s\", %d)", func, esc_key, (int)shift);
    DZFREE(esc_key);
    if (luaL_dostring(L, kstring_content(s)) != LUA_OK) {
        EPRINTF("Error in %s: %s", kstring_content(s), util_get_error(L));
        nerror = 1;
    }
    kstring_free(s);
    return nerror;
}

int get_configured_fps(lua_State* L) {
    lua_getglobal(L, "Vis");
    lua_getfield(L, -1, "FPS_LIMIT");
    int result = (int)lua_tointeger(L, -1);
    lua_pop(L, 2);
    return result;
}

int do_frames2msec(lua_State* L, fnum frame) {
    int fps = get_configured_fps(L);
    return (int)(frame * 1000.0 / fps + 0.5);
}

fnum do_msec2frames(lua_State* L, long msec) {
    int fps = get_configured_fps(L);
    return (fnum)(fps * (double)msec / 1000.0 + 0.5);
}

kstr lua_inspect_value(lua_State* L, int arg) {
    lua_getglobal(L, "require");
    lua_pushstring(L, "inspect");
    lua_call(L, 1, 1);
    lua_getfield(L, -1, "inspect");
    lua_pushvalue(L, arg);
    lua_call(L, 1, 1);
    kstr s = kstring_newfromvf("[%s]%s",
            lua_typename(L, lua_type(L, arg)), lua_tostring(L, -1));
    lua_pop(L, 2);
    return s;
}

emit_desc* lua_args_to_emit_desc(lua_State* L, int arg, fnum* when) {
    emit_desc* emit = emit_new();

    emit->n = luaL_checkint(L, arg++);
    if (when != NULL) {
        *when = do_msec2frames(L, luaL_checkunsigned(L, arg++));
    }

    /* behavior depends on the type of the next argument */
    int arg_type = lua_type(L, arg);
    if (arg_type != LUA_TTABLE) {
        luaL_error(L, "Invalid argument type; expected table");
        DZFREE(emit);
        return NULL;
    }

#if DEBUG >= DEBUG_TRACE
    int nargs = lua_gettop(L);
    kstr s = kstring_newfromvf("args[%d]: ", nargs);
    for (int argi = 1; argi <= nargs; ++argi) {
        kstr vs = lua_inspect_value(L, argi);
        kstring_appendvf(s, "%s, ", kstring_content(vs));
        kstring_free(vs);
    }
    DBPRINTF("Arguments: %s", kstring_content(s));
    kstring_free(s);
#endif

    /* defaults: circle, 100 msec, 1 pixel, white */
    emit->theta = emit->utheta = M_PI;
    emit->life = (int)do_msec2frames(L, 100);
    emit->r = emit->g = emit->b = 1.0f;
    emit->ur = emit->ug = emit->ub = 0.0f;
    emit->force = VIS_DEFAULT_FORCE;
    emit->limit = VIS_DEFAULT_LIMIT;
    emit->blender = VIS_DEFAULT_BLEND;

    int argi = lua_absindex(L, arg);
    lua_pushnil(L);
    while (lua_next(L, argi) != 0) {
        const char* key = lua_tostring(L, -2);
        int valtype = lua_type(L, -1);

        /* avert ye eyes, ye sensitive of constitution */

        if (!strcmp(key, "x")) {
            emit->x = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "y")) {
            emit->y = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "ux")) {
            emit->ux = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "uy")) {
            emit->uy = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "s")) {
            emit->s = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "us")) {
            emit->us = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "ds")) {
            emit->ds = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "uds")) {
            emit->uds = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "rad")) {
            emit->rad = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "urad")) {
            emit->urad = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "theta")) {
            emit->theta = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "utheta")) {
            emit->utheta = luaL_checknumber(L, -1);
        } else if (!strcmp(key, "life")) {
            emit->life = (int)do_msec2frames(L, luaL_checkunsigned(L, -1));
        } else if (!strcmp(key, "ulife")) {
            emit->ulife = (int)do_msec2frames(L, luaL_checkunsigned(L, -1));
        } else if (!strcmp(key, "r")) {
            emit->r = (float)luaL_checknumber(L, -1);
        } else if (!strcmp(key, "g")) {
            emit->g = (float)luaL_checknumber(L, -1);
        } else if (!strcmp(key, "b")) {
            emit->b = (float)luaL_checknumber(L, -1);
        } else if (!strcmp(key, "ur")) {
            emit->ur = (float)luaL_checknumber(L, -1);
        } else if (!strcmp(key, "ug")) {
            emit->ug = (float)luaL_checknumber(L, -1);
        } else if (!strcmp(key, "ub")) {
            emit->ub = (float)luaL_checknumber(L, -1);
        } else if (!strcmp(key, "force")) {
            emit->force = (force_id)luaL_checknumber(L, -1);
        } else if (!strcmp(key, "limit")) {
            emit->limit = (limit_id)luaL_checknumber(L, -1);
        } else if (!strcmp(key, "blender")) {
            emit->blender = (blend_id)luaL_checknumber(L, -1);
        } else if (!strcmp(key, "tag")) {
            arg_type = lua_type(L, -1);
            if (arg_type == LUA_TNUMBER) {
                emit->tag.ul = luaL_checkunsigned(L, -1);
            } else if (arg_type == LUA_TSTRING) {
                const char* value = luaL_checkstring(L, -1);
                emit->tag.ul = pextra_hash_string(value);
#if DEBUG >= DEBUG_TRACE
                DBPRINTF("Hashed string \"%s\" to %llx", value, emit->tag.ul);
#endif
            }
        } else {
            EPRINTF("Unknown key in emit table %s (type %s)",
                    key, lua_typename(L, valtype));
        }
        lua_pop(L, 1);
    }
    return emit;
}
/* end of private API */

/* start of Lua API */
/* Vis.debug(Vis.flist, ...) */
int viscmd_debug_fn(lua_State* L) {
    int nargs = lua_gettop(L);
    lua_Debug ar;
    lua_getstack(L, 1, &ar);
    lua_getinfo(L, "nSl", &ar);
    kstr s = kstring_newfromvf("args[%d]{ ", nargs);
    for (int argi = 1; argi <= nargs; ++argi) {
        kstr vs = lua_inspect_value(L, argi);
        if (argi > 1) {
            kstring_append(s, ", ");
        }
        kstring_append(s, kstring_content(vs));
        kstring_free(vs);
    }
    kstring_appendvf(s, "%s", " }");
#if DEBUG >= DEBUG_DEBUG
    DBPRINTF("(function %s)[%s:%d]: Vis.debug(%s)", ar.name, ar.source,
            ar.currentline, kstring_content(s));
#else
    fprintf(stderr, "%s:%d: Debug: %s", ar.source, ar.currentline,
            kstring_content(s));
#endif
    kstring_free(s);
    return 0;
}

/* Vis.command(Vis.flist, when, "command") */
int viscmd_command_fn(lua_State* L) {
    flist_t fl = *(flist_t*)luaL_checkudata(L, 1, "flist**");
    fnum when = do_msec2frames(L, luaL_checkunsigned(L, 2));
    const char* cmd = luaL_checkstring(L, 3);
    /* DBPRINTF("command(%p, %d, \"%s\")", fl, when, cmd); */
    flist_insert_cmd(fl, when, cmd);
    return 0;
}

/* Vis.exit(Vis.flist, when) */
int viscmd_exit_fn(lua_State* L) {
    flist_t fl = *(flist**)luaL_checkudata(L, 1, "flist**");
    fnum when = do_msec2frames(L, luaL_checkunsigned(L, 2));
    flist_insert_exit(fl, when);
    return 0;
}

/* Vis.emit(Vis.flist, n, when, x, y, [ux, uy, radius, uradius, ds, uds,
 *          theta, utheta, life, ulife, r, g, b, ur, ug, ub, force, limit,
 *          blender]),
 * particle.x = x + random(-ux, ux)
 * particle.y = y + random(-uy, uy)
 * and so on for all <arg>, u<arg> pairs.
 * 0 <= r, g, b, ur, ug, ub <= 1
 * @param radius defaults to 1
 * color (r, g, b) defaults to 1
 * @param force defaults to no passive force
 * @param limit defaults to no limit fn
 * @param blender defaults to linear blend (fade to black) */
int viscmd_emit_fn(lua_State* L) {
    int arg = 1;
    fnum when;
    flist_t fl = *(flist**)luaL_checkudata(L, arg++, "flist**");
    emit_desc* frame = lua_args_to_emit_desc(L, arg, &when);
    flist_insert_emit(fl, when, frame);
    return 0;
}

/* Vis.audio(Vis.flist, when, path) */
int viscmd_audio_fn(lua_State* L) {
    flist_t fl = *(flist**)luaL_checkudata(L, 1, "flist**");
    fnum when = do_msec2frames(L, luaL_checkunsigned(L, 2));
    const char* file = luaL_checkstring(L, 3);
    if (!audio_open(file)) {
        DBPRINTF("Vis.audio(%s) failed", file);
        lua_pushnil(L);
    } else {
        DBPRINTF("Vis.audio(%s) loaded", file);
        flist_insert_play(fl, when);
        lua_pushboolean(L, TRUE);
    }
    return 1;
}

/* Vis.play() */
int viscmd_play_fn(UNUSED_PARAM(lua_State* L)) {
    DBPRINTF("%s", "Vis.play()");
    audio_play();
    return 0;
}

/* Vis.pause() */
int viscmd_pause_fn(UNUSED_PARAM(lua_State* L)) {
    DBPRINTF("%s", "Vis.pause()");
    audio_pause();
    return 0;
}

/* Vis.volume(level) */
int viscmd_volume_fn(lua_State* L) {
    float level = (float)luaL_checknumber(L, 1);
    DBPRINTF("Vis.volume(%f)", level);
    audio_set_volume(level);
    return 0;
}

/* Vis.seek(offset),
 * @param offset is in 100ths of a second, NOT milliseconds */
int viscmd_seek_fn(lua_State* L) {
    unsigned offset = luaL_checkunsigned(L, 1);
    DBPRINTF("Vis.seek(%d)", (int)offset);
    audio_seek(offset);
    return 0;
}

/* Vis.seekms(Vis.flist, when, whereto) */
int viscmd_seekms_fn(lua_State* L) {
    flist_t fl = *(flist**)luaL_checkudata(L, 1, "flist**");
    fnum where = do_msec2frames(L, luaL_checkunsigned(L, 2));
    fnum whereto = do_msec2frames(L, luaL_checkunsigned(L, 3));
    DBPRINTF("Vis.seekms(%p, [frame]%d, [frame]%d)", fl, where, whereto);
    flist_insert_seekframe(fl, where, whereto);
    return 0;
}

/* Vis.seekframe(Vis.flist, when, whereto) */
int viscmd_seekframe_fn(lua_State* L) {
    flist_t fl = *(flist**)luaL_checkudata(L, 1, "flist**");
    fnum where = (fnum)luaL_checkunsigned(L, 2);
    fnum whereto = (fnum)luaL_checkunsigned(L, 3);
    DBPRINTF("Vis.seekframe(%p, [frame]%d, [frame]%d)", fl, where, whereto);
    flist_insert_seekframe(fl, where, whereto);
    return 0;
}

/* Vis.gotoframe(Vis.flist, whereto) */
int viscmd_gotoframe_fn(lua_State* L) {
    flist_t fl = *(flist**)luaL_checkudata(L, 1, "flist**");
    fnum whereto = (fnum)luaL_checkunsigned(L, 2);
    DBPRINTF("Vis.gotoframe(%p, [frame]%d)", fl, whereto);
    flist_goto_frame(fl, whereto);
    return 0;
}

/* Vis.audiosync(Vis.flist, when, nframes) */
int viscmd_audiosync_fn(lua_State* L) {
    flist_t fl = *(flist**)luaL_checkudata(L, 1, "flist**");
    fnum where = (fnum)luaL_checkunsigned(L, 2);
    fnum nframes = (fnum)luaL_checkunsigned(L, 3);
    DBPRINTF("Vis.audiosync(%p, [frame]%d, [frame]%d)", fl, where, nframes);
    flist_insert_audiosync(fl, where, nframes);
    return 0;
}

/* Vis.delay(Vis.flist, when, length) */
static int viscmd_delay_fn(lua_State* L) {
    flist_t fl = *(flist**)luaL_checkudata(L, 1, "flist**");
    fnum where = (fnum)luaL_checkunsigned(L, 2);
    fnum length = (fnum)luaL_checkunsigned(L, 3);
    DBPRINTF("Vis.delay(%p, [frame]%d, [frame]%d)", fl, where, length);
    flist_insert_delay(fl, where, length);
    return 0;
}

/* Vis.bgcolor(Vis.flist, when, r, g, b),
 * 0 <= @param rgb <= 1 */
int viscmd_bgcolor_fn(lua_State* L) {
    flist_t fl = *(flist**)luaL_checkudata(L, 1, "flist**");
    fnum when = do_msec2frames(L, luaL_checkunsigned(L, 2));
    float c[3] = {
        (float)luaL_checknumber(L, 3),
        (float)luaL_checknumber(L, 4),
        (float)luaL_checknumber(L, 5)
    };
    flist_insert_bgcolor(fl, when, c);
    if (DEBUG > DEBUG_DEBUG) {
        DBPRINTF("Vis.bgcolor(%p, %d, %g, %g, %g)", fl, when, c[0], c[1], c[2]);
    }
    return 0;
}

/* See README.md for mutate arguments
 * <mutate-optional-args>: factor, offset, offset
 * Vis.mutate(Vis.flist, when, func, factor[, <mutate-optional-args>])
 * Vis.mutate(Vis.flist, when, func, tag)
 * Vis.mutate(Vis.flist, when, func, cond[, tag[, factor[, <mutate-optional-args>)
 * @param func is a valid Vis.MUTATE_*
 * @param cond is a valid Vis.MUTATE_IF_* */
int viscmd_mutate_fn(lua_State* L) {
    mutate_method* method = DBMALLOC(sizeof(struct mutate_method));
    flist_t fl = *(flist**)luaL_checkudata(L, 1, "flist**");
    fnum when = do_msec2frames(L, luaL_checkunsigned(L, 2));
    mutate_id fnid = (mutate_id)luaL_checkint(L, 3);
    method->id = fnid;
    if (fnid >= VIS_MUTATE_PUSH && fnid < VIS_MUTATE_TAG_SET) {
        /* case 1: normal mutate */
        method->factor[0] = luaL_checknumber(L, 4);
        method->factor[1] = luaL_optnumber(L, 5, 0);
        method->offset[0] = luaL_optnumber(L, 6, 0);
        method->offset[1] = luaL_optnumber(L, 7, 0);
        DBPRINTF("Vis.mutate(%p, %d, %s, %g, %g, %f, %f)",
                fl, (int)when, genlua_mutate(fnid),
                method->factor[0], method->factor[1],
                method->offset[0], method->offset[1]);
    } else if (fnid >= VIS_MUTATE_TAG_SET && fnid < VIS_MUTATE_PUSH_IF) {
        /* case 2: tag modification */
        if (lua_type(L, 4) == LUA_TNUMBER) {
            method->tag.i.l = luaL_checkint(L, 4);
        }
        DBPRINTF("Vis.mutate(%p, %d, %s, %d)",
                fl, (int)when, genlua_mutate(fnid), method->tag.i.l);
    } else if (fnid >= VIS_MUTATE_PUSH_IF && fnid < VIS_NMUTATES) {
        /* case 3: conditional mutate */
        method->factor[0] = luaL_checknumber(L, 4);
        method->cond = (mutate_cond_id)luaL_checkint(L, 5);
        if (lua_type(L, 6) == LUA_TNUMBER) {
            method->tag.i.l = luaL_checkint(L, 6);
        }
        method->factor[1] = luaL_optnumber(L, 7, 0);
        method->offset[0] = luaL_optnumber(L, 8, 0);
        method->offset[1] = luaL_optnumber(L, 9, 0);
        DBPRINTF("Vis.mutate(%p, %d, %s, %g, %s, %d, %g)",
                fl, (int)when, genlua_mutate(fnid), method->factor[0],
                genlua_mutate_cond(method->cond), method->tag.i.l,
                method->factor[1]);
    } else {
        DZFREE(method);
        return luaL_error(L, "Invalid mutate ID %d", fnid);
    }
    method->func = MUTATE_MAP[fnid];
    flist_insert_mutate(fl, when, method);
    return 0;
}

/* TODO: Vis.callback(Vis.flist, when, Vis.script, func, ...args) */
/* Vis.callback(Vis.flist, when, Vis.script, "lua") */
int viscmd_callback_fn(lua_State* L) {
    flist_t fl = *(flist**)luaL_checkudata(L, 1, "flist**");
    fnum when = do_msec2frames(L, luaL_checkunsigned(L, 2));
    script_t s = util_checkscript(L, 3);
    script_cb* scb = DBMALLOC(sizeof(struct script_cb));
    scb->owner = s;
    scb->fn_name = dupstr("<lua>");
    scb->fn_code = escape_string(luaL_checkstring(L, 4));
    DBPRINTF("Vis.callback(%p, %d, %p, %s)", fl, when, s, scb->fn_code);
    flist_insert_scriptcb(fl, when, scb);
    return 0;
}

/* fps = Vis.fps(Vis.script) */
int viscmd_fps_fn(lua_State* L) {
    script_t s = util_checkscript(L, 1);
    drawer_t drawer = util_checkdrawer(L, s);
    lua_pushnumber(L, (lua_Number)drawer_get_fps(drawer));
    return 1;
}

/* Vis.settrace(Vis.script, ...)
 * Function has same args as Vis.emit except:
 *      @param Vis.flist is replaced with Vis.script
 *      @param when is omitted */
int viscmd_settrace_fn(lua_State* L) {
    script_t s = util_checkscript(L, 1);
    drawer_t drawer = util_checkdrawer(L, s);
    drawer_set_trace(drawer, lua_args_to_emit_desc(L, 2, NULL));
    return 0;
}

/* Vis.emitnow(Vis.script, ...)
 * Function has the same args as Vis.settrace */
int viscmd_emitnow_fn(lua_State* L) {
    /* Vis.script is actually ignored, but is present for consistency */
    emit_desc* emit = lua_args_to_emit_desc(L, 2, NULL);
    emit_frame(emit);
    emit_free(emit);
    return 0;
}

/* Vis.frames2msec(frame) */
int viscmd_f2ms_fn(lua_State* L) {
    int result = do_frames2msec(L, (fnum)luaL_checknumber(L, 1));
    lua_pushinteger(L, result);
    return 1;
}

/* Vis.msec2frames(msec) */
int viscmd_ms2f_fn(lua_State* L) {
    fnum result = do_msec2frames(L, (long)luaL_checknumber(L, 1));
    lua_pushinteger(L, result);
    return 1;
}

/* Vis.get_debug(Vis.script, what) */
int viscmd_get_debug_fn(lua_State* L) {
    script_t s = util_checkscript(L, 1);
    const char* what = luaL_checkstring(L, 2);
    if (!strcmp(what, "PARTICLES-EMITTED")) {
        lua_pushunsigned(L, (uint32_t)s->dbg->particles_emitted);
    } else if (!strcmp(what, "TIME-NOW")) {
        lua_pushunsigned(L, (uint32_t)s->dbg->time_now);
    } else if (!strcmp(what, "FRAMES-EMITTED")) {
        lua_pushunsigned(L, (uint32_t)s->dbg->frames_emitted);
    } else if (!strcmp(what, "NUM-MUTATES")) {
        lua_pushunsigned(L, (uint32_t)s->dbg->num_mutates);
    } else if (!strcmp(what, "PARTICLES-MUTATED")) {
        lua_pushunsigned(L, (uint32_t)s->dbg->particles_mutated);
    } else if (!strcmp(what, "PARTICLE-TAGS-MODIFIED")) {
        lua_pushunsigned(L, (uint32_t)s->dbg->particle_tags_modified);
    } else if (!strcmp(what, "FRAME-EMIT-COUNTS")) {
        lua_createtable(L, VIS_MAX_FTYPE, 0);
        for (ftype_id i = (ftype_id)0; i < VIS_MAX_FTYPE; ++i) {
            lua_pushunsigned(L, i);
            lua_pushunsigned(L, emitter_get_frame_count(i));
            lua_settable(L, -3);
        }
    } else {
        s->errors += 1;
        return luaL_error(L, "Debug token \"%s\" invalid", what);
    }
    return 1;
}
/* end of Lua API */
