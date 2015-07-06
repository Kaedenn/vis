
#define _BSD_SOURCE /* for setenv */

#include "audio.h"
#include "helper.h"
#include "flist.h"
#include "script.h"
#include "mutator.h"
#include "kstring.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static script_t luautil_checkscript(lua_State* L, int pos);

static int viscmd_debug_fn(lua_State* L);
static int viscmd_command_fn(lua_State* L);
static int viscmd_emit_fn(lua_State* L);
static int viscmd_audio_fn(lua_State* L);
static int viscmd_play_fn(lua_State* L);
static int viscmd_pause_fn(lua_State* L);
static int viscmd_seek_fn(lua_State* L);
static int viscmd_seekms_fn(lua_State* L);
static int viscmd_seekframe_fn(lua_State* L);
static int viscmd_bgcolor_fn(lua_State* L);
static int viscmd_mutate_fn(lua_State* L);
static int viscmd_callback_fn(lua_State* L);
static int viscmd_fps_fn(lua_State* L);
static int viscmd_settrace_fn(lua_State* L);

static int viscmd_f2ms_fn(lua_State* L);
static int viscmd_ms2f_fn(lua_State* L);

static emit_t lua_args_to_emit_t(lua_State* L, int arg, fnum_t* when);

struct script {
    lua_State* L;
    flist_t fl;
    drawer_t drawer;
};

char* genlua_emit(emit_t emit, fnum_t when) {
    char* result;
    kstr s = kstring_newfrom("Vis.emit(");
    kstring_append(s, "Vis.flist, ");
    kstring_appendvf(s, "%d, Vis.frames2msec(%d), ", emit->n, when);
    kstring_appendvf(s, "%g, %g, %g, %g, ", emit->x, emit->y, emit->ux,
                     emit->uy);
    kstring_appendvf(s, "%g, %g, ", emit->rad, emit->urad);
    kstring_appendvf(s, "%g, %g, ", emit->ds, emit->uds);
    kstring_appendvf(s, "%g, %g, ", emit->theta, emit->utheta);
    kstring_appendvf(s, "Vis.frames2msec(%d), Vis.frames2msec(%d), ",
                     emit->life, emit->ulife);
    kstring_appendvf(s, "%g, %g, %g, ", (double)emit->r, (double)emit->g,
                     (double)emit->b);
    kstring_appendvf(s, "%g, %g, %g, ", (double)emit->ur, (double)emit->ug,
                     (double)emit->ub);
    kstring_appendvf(s, "%s, ", genlua_force(emit->force));
    kstring_appendvf(s, "%s, ", genlua_limit(emit->limit));
    kstring_appendvf(s, "%s)", genlua_blender(emit->blender));

    result = dupstr(kstring_content(s));
    kstring_free(s);
    return result;
}

const char* genlua_force(force_id force) {
    switch (force) {
        case VIS_DEFAULT_FORCE: return "Vis.DEFAULT_FORCE";
        case VIS_FORCE_FRICTION: return "Vis.FORCE_FRICTION";
        case VIS_FORCE_GRAVITY: return "Vis.FORCE_GRAVITY";
        case VIS_NFORCES: return "Vis.NFORCES";
        default: return "nil";
    }
}

const char* genlua_limit(limit_id limit) {
    switch (limit) {
        case VIS_DEFAULT_LIMIT: return "Vis.DEFAULT_LIMIT";
        case VIS_LIMIT_BOX: return "Vis.LIMIT_BOX";
        case VIS_LIMIT_SPRINGBOX: return "Vis.LIMIT_SPRINGBOX";
        case VIS_NLIMITS: return "Vis.NLIMITS";
        default: return "nil";
    }
}

const char* genlua_blender(blend_id blender) {
    switch (blender) {
        case VIS_BLEND_NONE: return "Vis.BLEND_NONE";
        /* case VIS_DEFAULT_BLEND: */
        case VIS_BLEND_LINEAR: return "Vis.BLEND_LINEAR";
        case VIS_BLEND_QUADRATIC: return "Vis.BLEND_QUADRATIC";
        case VIS_BLEND_NEGGAMMA: return "Vis.BLEND_NEGGAMMA";
        case VIS_NBLENDS: return "Vis.NBLENDS";
        default: return "nil";
    }
}

int initialize_vis_lib(lua_State* L) {
    static const struct luaL_Reg vis_lib[] = {
        {"debug", viscmd_debug_fn},
        {"command", viscmd_command_fn},
        {"emit", viscmd_emit_fn},
        {"audio", viscmd_audio_fn},
        {"play", viscmd_play_fn},
        {"pause", viscmd_pause_fn},
        {"seek", viscmd_seek_fn},
        {"seekms", viscmd_seekms_fn},
        {"seekframe", viscmd_seekframe_fn},
        {"bgcolor", viscmd_bgcolor_fn},
        {"mutate", viscmd_mutate_fn},
        {"callback", viscmd_callback_fn},
        {"fps", viscmd_fps_fn},
        {"settrace", viscmd_settrace_fn},
        {"frames2msec", viscmd_f2ms_fn},
        {"msec2frames", viscmd_ms2f_fn},
        {NULL, NULL}
    };

    luaL_newlib(L, vis_lib);

#define NEW_CONST(name) \
    lua_pushliteral(L, #name); \
    lua_pushnumber(L, VIS_##name); \
    lua_settable(L, -3)

    /* grant access to all of the enums and constants */
    NEW_CONST(FPS_LIMIT);
    NEW_CONST(WIDTH);
    NEW_CONST(HEIGHT);
    /* blenders */
    NEW_CONST(DEFAULT_BLEND);
    NEW_CONST(BLEND_NONE);
    NEW_CONST(BLEND_LINEAR);
    NEW_CONST(BLEND_QUADRATIC);
    NEW_CONST(BLEND_NEGGAMMA);
    NEW_CONST(NBLENDS);
    /* forces */
    NEW_CONST(DEFAULT_FORCE);
    NEW_CONST(FORCE_FRICTION);
    NEW_CONST(FORCE_GRAVITY);
    NEW_CONST(NFORCES);
    /* limits */
    NEW_CONST(DEFAULT_LIMIT);
    NEW_CONST(LIMIT_BOX);
    NEW_CONST(LIMIT_SPRINGBOX);
    NEW_CONST(NLIMITS);
    /* mutators */
    NEW_CONST(MUTATE_PUSH);
    NEW_CONST(MUTATE_SLOW);
    NEW_CONST(MUTATE_SHRINK);
    NEW_CONST(MUTATE_GROW);
    NEW_CONST(MUTATE_PUSH_DX);
    NEW_CONST(MUTATE_PUSH_DY);
    NEW_CONST(MUTATE_AGE);
    NEW_CONST(MUTATE_OPACITY);
    NEW_CONST(NMUTATES);
    /* other constants */
    NEW_CONST(FORCE_FRICTION_COEFF);
    NEW_CONST(FORCE_GRAVITY_FACTOR);
#undef NEW_CONST

    return 1;
}

script_t script_new(script_cfg_t cfg) {
    script_t s = DBMALLOC(sizeof(struct script));
    s->fl = flist_new();
    s->L = luaL_newstate();
    luaL_openlibs(s->L);
    luaL_requiref(s->L, "Vis", initialize_vis_lib, 0);
    lua_pop(s->L, 1);

    /* adjust lua search path, include ./lua and ./test,
     * include default modules */
    (void)luaL_dostring(s->L,
        "package = require('package')\n"
        "package.path = '; ;./?.lua;./lua/?.lua;./test/?.lua'"
        "Vis = require(\"Vis\")\n"
        "VisUtil = require(\"visutil\")\n"
        "Vis.on_mousedown = function() end\n"
        "Vis.on_mouseup = function() end\n"
        "Vis.on_mousemove = function() end\n"
        "Vis.on_keydown = function() end\n"
        "Vis.on_keyup = function() end\n"
        "Vis.on_quit = function() end\n"
    );
    if (file_exists(LUA_STARTUP_FILE)) {
        DBPRINTF("Executing startup file: %s", LUA_STARTUP_FILE);
        (void)luaL_dofile(s->L, LUA_STARTUP_FILE);
    }

    lua_getglobal(s->L, "Vis");
    
    flist_t* flbox = lua_newuserdata(s->L, sizeof(flist_t));
    luaL_newmetatable(s->L, "flist_t*");
    lua_pop(s->L, 1);
    luaL_setmetatable(s->L, "flist_t*");
    *flbox = s->fl;
    lua_setfield(s->L, -2, "flist");
    if ((cfg & SCRIPT_NO_CB) == 0) {
        script_t* sbox = lua_newuserdata(s->L, sizeof(script_t));
        luaL_newmetatable(s->L, "script_t*");
        lua_pop(s->L, 1);
        luaL_setmetatable(s->L, "script_t*");
        *sbox = s;
        lua_setfield(s->L, -2, "script");
    }

#ifdef notyet
    struct emit* box = lua_newuserdata(s->L, sizeof(struct emit));
    luaL_newmetatable(s->L, "emit*");
    lua_newtable(s->L);
    /* pushcfunction, setfield, repeat */
    lua_setfield(s->L, -2, "__index");
    luaL_setmetatable(s->L, "emit*");
#endif

    lua_pop(s->L, 1); /* getglobal("Vis") */

    return s;
}

void script_free(script_t s) {
    if (!s) return;
    lua_close(s->L);
    DBFREE(s);
}

flist_t script_run(script_t s, const char* filename) {
    /* s->fl already bound to script in script_new */
    DBPRINTF("Running %s to build %p", filename, s->fl);
    if (luaL_dofile(s->L, filename) != LUA_OK) {
        eprintf("Error: %s", lua_tostring(s->L, -1));
    }
    return s->fl;
}

void script_run_string(script_t script, const char* torun) {
    if (luaL_dostring(script->L, torun) != LUA_OK) {
        eprintf("script \"%s\" error: %s", torun, lua_tostring(script->L, -1));
    }
}

void call_script(script_t s, script_cb_t cb, UNUSED_PARAM(void* args)) {
    if (luaL_dostring(s->L, cb->fn_code) != LUA_OK) {
        eprintf("callback error: %s", lua_tostring(s->L, -1));
    }
}

void script_set_drawer(script_t script, drawer_t drawer) {
    script->drawer = drawer;
}

void script_callback_free(script_cb_t cb) {
    DBFREE(cb->fn_name);
    DBFREE(cb->fn_code);
    DBFREE(cb);
}

static void do_mouse_event(lua_State* L, const char* func, int x, int y) {
    kstr s = kstring_newfromvf("Vis.on_%s(%d, %d)", func, x, y);
    if (luaL_dostring(L, kstring_content(s)) != LUA_OK) {
        eprintf("Unknown error in %s", kstring_content(s));
    }
    kstring_free(s);
}

void script_mousemove(script_t script, int x, int y) {
    do_mouse_event(script->L, "mousemove", x, y);
}

void script_mousedown(script_t script, int x, int y) {
    do_mouse_event(script->L, "mousedown", x, y);
}

void script_mouseup(script_t script, int x, int y) {
    do_mouse_event(script->L, "mouseup", x, y);
}

static void do_keyboard_event(lua_State* L, const char* func, const char* key) {
    char* esc_key = escape_string(key);
    kstr s = kstring_newfromvf("Vis.on_%s(\"%s\")", func, esc_key);
    if (luaL_dostring(L, kstring_content(s)) != LUA_OK) {
        eprintf("Unknown error in %s", kstring_content(s));
    }
    kstring_free(s);
    free(esc_key);
}

void script_keydown(script_t script, const char* keyname) {
    do_keyboard_event(script->L, "keydown", keyname);
}

void script_keyup(script_t script, const char* keyname) {
    do_keyboard_event(script->L, "keyup", keyname);
}

void script_quit(script_t script) {
    if (luaL_dostring(script->L, "Vis.on_quit()") != LUA_OK) {
        eprintf("Unknown error in %s", "Vis.on_quit()");
    }
}

/* end of public API */
script_t luautil_checkscript(lua_State* L, int arg) {
    if (lua_isnil(L, arg)) {
        luaL_error(L, "expected Vis.script, received nil, "
                      "function may be disabled");
        return NULL;
    } else {
        return *(script_t*)luaL_checkudata(L, arg++, "script_t*");
    }
}

/* Vis.debug(Vis.flist, ...) */
int viscmd_debug_fn(lua_State* L) {
    int nargs = lua_gettop(L);
    lua_Debug ar;
    lua_getstack(L, 1, &ar);
    lua_getinfo(L, "nSl", &ar);
    kstr s = kstring_newfromvf("%d", nargs);
    for (int i = 1; i <= nargs; ++i) {
        switch (lua_type(L, i)) {
            case LUA_TNIL:
                kstring_appendvf(s, ", %s", "nil");
                break;
            case LUA_TNUMBER:
                kstring_appendvf(s, ", %g", luaL_checknumber(L, i));
                break;
            case LUA_TBOOLEAN:
                kstring_appendvf(s, ", %s",
                                 luaL_checkint(L, i) ? "true" : "false");
                break;
            case LUA_TSTRING: {
                char* esc = escape_string(luaL_checkstring(L, i));
                kstring_appendvf(s, ", \"%s\"", esc);
                DBFREE(esc);
            } break;
            case LUA_TTABLE: /* TODO: dump a table */
            case LUA_TFUNCTION:
            case LUA_TUSERDATA:
            case LUA_TTHREAD:
            case LUA_TLIGHTUSERDATA:
            default:
                kstring_appendvf(s, ", <%s>", lua_typename(L, lua_type(L, i)));
                break;
         }
    }
    DBPRINTF("(function %s)[%d]: Vis.debug(%s)", ar.name, ar.currentline,
             kstring_content(s));
    kstring_free(s);
    return 0;
}

/* Vis.command(Vis.flist, when, "command") */
int viscmd_command_fn(lua_State* L) {
    flist_t fl = *(flist_t *)luaL_checkudata(L, 1, "flist_t*");
    fnum_t when = (fnum_t)VIS_MSEC_TO_FRAMES(luaL_checkunsigned(L, 2));
    const char* cmd = luaL_checkstring(L, 3);
    DBPRINTF("command(%p, %d, \"%s\")", fl, when, cmd);
    flist_insert_cmd(fl, when, cmd);
    return 0;
}

/* Vis.emit(Vis.flist, n, when, x, y, [ux, uy, radius, uradius, ds, uds,
 *          theta, utheta, life, ulife, r, g, b, ur, ug, ub, force, limit,
 *          blender]),
 * particle.x = x + random(-ux, ux)
 * particle.y = y + random(-uy, uy)
 * and so on for all <arg>, u<arg> pairs.
 * 0 <= r, g, b, ur, ug, ub <= 1,
 * @param radius defaults to 1
 * color (r, g, b) defaults to 1
 * @param force defaults to no passive force
 * @param limit defaults to no limit fn
 * @param blender defaults to linear blend (fade to black) */
int viscmd_emit_fn(lua_State* L) {
    int arg = 1;
    fnum_t when;
    flist_t fl = *(flist_t*)luaL_checkudata(L, arg++, "flist_t*");
    emit_t frame = lua_args_to_emit_t(L, arg, &when);
    flist_insert_emit(fl, when, frame);
    return 0;
}

/* Vis.audio(path),
 * @param path must resolve to a .WAV file */
int viscmd_audio_fn(lua_State* L) {
    const char* file = luaL_checkstring(L, 1);
    if (!audio_open(file)) {
        DBPRINTF("Vis.audio(%s) failed", file);
        lua_pushnil(L);
    } else {
        DBPRINTF("Vis.audio(%s) loaded, playing", file);
        audio_play();
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

/* Vis.seek(offset),
 * @param offset is in milliseconds */
int viscmd_seek_fn(lua_State* L) {
    unsigned offset = luaL_checkunsigned(L, 1);
    DBPRINTF("Vis.seek(%d)", (int)offset);
    audio_seek(offset);
    return 0;
}

/* Vis.seekms(Vis.flist, when, whereto) */
int viscmd_seekms_fn(lua_State* L) {
    flist_t fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    fnum_t where = (fnum_t)VIS_MSEC_TO_FRAMES(luaL_checkunsigned(L, 2));
    fnum_t whereto = (fnum_t)VIS_MSEC_TO_FRAMES(luaL_checkunsigned(L, 3));
    DBPRINTF("Vis.seekms(%p, (frames)%d, (frames)%d)", fl, where, whereto);
    flist_insert_seekframe(fl, where, whereto);
    return 0;
}

/* Vis.seekframe(Vis.flist, when, whereto) */
int viscmd_seekframe_fn(lua_State* L) {
    flist_t fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    fnum_t where = (fnum_t)luaL_checkunsigned(L, 2);
    fnum_t whereto = (fnum_t)luaL_checkunsigned(L, 3);
    DBPRINTF("Vis.seekframe(%p, %d, %d)", fl, where, whereto);
    flist_insert_seekframe(fl, where, whereto);
    return 0;
}

/* Vis.bgcolor(Vis.script, r, g, b),
 * 0 <= @param rgb <= 1 */
int viscmd_bgcolor_fn(lua_State* L) {
    script_t s = luautil_checkscript(L, 1);
    if (s->drawer == NULL) {
        return luaL_error(L, "drawer is NULL; function is disabled");
    }
    float r = (float)luaL_optnumber(L, 2, 0);
    float g = (float)luaL_optnumber(L, 3, 0);
    float b = (float)luaL_optnumber(L, 4, 0);
    drawer_bgcolor(s->drawer, r, g, b);
    DBPRINTF("Vis.bgcolor(%p, %g, %g, %g)", s->drawer, (double)r, (double)g,
             (double)b);
    return 0;
}

/* Vis.mutate(Vis.flist, when, func, factor),
 * @param func is a valid Vis.MUTATE_* */
int viscmd_mutate_fn(lua_State* L) {
    mutate_method_t method = DBMALLOC(sizeof(struct mutate_method));
    flist_t fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    fnum_t when = (fnum_t)VIS_MSEC_TO_FRAMES(luaL_checkunsigned(L, 2));
    mutate_id fnid = (mutate_id)luaL_checkint(L, 3);
    double factor = (double)luaL_checknumber(L, 4);
    if (fnid < (mutate_id)0) fnid = (mutate_id)0;
    if (fnid >= VIS_NMUTATES) fnid = (mutate_id)0;
    method->func = MUTATE_MAP[fnid];
    method->factor = factor;
    DBPRINTF("Vis.mutate(%p, %d, {%d, %g})", fl, when, fnid, factor);
    flist_insert_mutate(fl, when, method);
    return 0;
}

/* Vis.callback(Vis.flist, when, Vis.script, "code") */
int viscmd_callback_fn(lua_State* L) {
    flist_t fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    fnum_t when = (fnum_t)VIS_MSEC_TO_FRAMES(luaL_checkunsigned(L, 2));
    script_t s = luautil_checkscript(L, 3);
    script_cb_t scb = DBMALLOC(sizeof(struct script_cb));
    scb->owner = s;
    scb->fn_name = dupstr("<lua>");
    scb->fn_code = escape_string(luaL_checkstring(L, 4));
    DBPRINTF("Vis.callback(%p, %d, %p, %s)", fl, when, s, scb->fn_code);
    flist_insert_scriptcb(fl, when, scb);
    return 0;
}

/* fps = Vis.fps(Vis.script) */
int viscmd_fps_fn(lua_State* L) {
    int arg = 1;
    script_t s = luautil_checkscript(L, arg++);
    if (s->drawer == NULL) {
        return luaL_error(L, "drawer is NULL; function is disabled");
    } else {
        lua_pushnumber(L, (lua_Number)drawer_get_fps(s->drawer));
    }
    return 1;
}

/* Vis.settrace(Vis.script, ...)
 * Function has same args as Vis.emit except:
 *      @param Vis.flist is replaced with Vis.script
 *      @param when is omitted */
int viscmd_settrace_fn(lua_State* L) {
    int arg = 1;
    script_t s = luautil_checkscript(L, arg++);
    if (s->drawer == NULL) {
        return luaL_error(L, "drawer is NULL; function is disabled");
    } else {
        drawer_set_trace(s->drawer, lua_args_to_emit_t(L, arg, NULL));
    }
    return 0;
}

/* Vis.frames2msec(frame) */
int viscmd_f2ms_fn(lua_State* L) {
    lua_pushinteger(L, VIS_FRAMES_TO_MSEC(luaL_checknumber(L, 1)));
    return 1;
}

/* Vis.msec2frames(msec) */
int viscmd_ms2f_fn(lua_State* L) {
    lua_pushinteger(L, VIS_MSEC_TO_FRAMES(luaL_checknumber(L, 1)));
    return 1;
}

static emit_t lua_args_to_emit_t(lua_State* L, int arg, fnum_t* when) {
    emit_t emit = DBMALLOC(sizeof(struct emit));
    emit->n = luaL_checkint(L, arg++);
    if (when != NULL) {
        *when = (fnum_t)VIS_MSEC_TO_FRAMES(luaL_checkunsigned(L, arg++));
    }
    emit->x = luaL_checknumber(L, arg++);
    emit->y = luaL_checknumber(L, arg++);
    emit->ux = luaL_optnumber(L, arg++, 0);
    emit->uy = luaL_optnumber(L, arg++, 0);
    emit->rad = luaL_optnumber(L, arg++, 1);
    emit->urad = luaL_optnumber(L, arg++, 0);
    emit->ds = luaL_optnumber(L, arg++, 0);
    emit->uds = luaL_optnumber(L, arg++, 0);
    emit->theta = luaL_optnumber(L, arg++, 0);
    emit->utheta = luaL_optnumber(L, arg++, 0);
    emit->life = VIS_MSEC_TO_FRAMES(luaL_optint(L, arg++, 100));
    emit->ulife = VIS_MSEC_TO_FRAMES(luaL_optint(L, arg++, 0));
    emit->r = (float)luaL_optnumber(L, arg++, 1.0);
    emit->g = (float)luaL_optnumber(L, arg++, 1.0);
    emit->b = (float)luaL_optnumber(L, arg++, 1.0);
    emit->ur = (float)luaL_optnumber(L, arg++, 0.0);
    emit->ug = (float)luaL_optnumber(L, arg++, 0.0);
    emit->ub = (float)luaL_optnumber(L, arg++, 0.0);
    emit->force = luaL_optint(L, arg++, VIS_DEFAULT_FORCE);
    emit->limit = luaL_optint(L, arg++, VIS_DEFAULT_LIMIT);
    emit->blender = luaL_optint(L, arg++, VIS_BLEND_LINEAR);
    return emit;
}

