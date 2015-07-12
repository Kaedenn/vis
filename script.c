
#define _BSD_SOURCE /* for setenv */

#include "audio.h"
#include "helper.h"
#include "flist.h"
#include "script.h"
#include "mutator.h"
#include "kstring.h"
#include "emitter.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

static script_t luautil_checkscript(lua_State* L, int pos);
static const char* luautil_get_error(lua_State* L);

static int viscmd_debug_fn(lua_State* L);
static int viscmd_command_fn(lua_State* L);
static int viscmd_exit_fn(lua_State* L);
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
static int viscmd_emitnow_fn(lua_State* L);
static int viscmd_f2ms_fn(lua_State* L);
static int viscmd_ms2f_fn(lua_State* L);
static int viscmd_get_debug_fn(lua_State* L);

static int do_mouse_event(lua_State* L, const char* func, int x, int y);
static int do_keyboard_event(lua_State* L, const char* func, const char* key,
                             BOOL shift);
static emit_t lua_args_to_emit_t(lua_State* L, int arg, fnum_t* when);

struct script {
    script_debug_t dbg;
    lua_State* L;
    flist_t fl;
    drawer_t drawer;
    int errors;
};

int initialize_vis_lib(lua_State* L) {
    static const struct luaL_Reg vis_lib[] = {
        {"debug", viscmd_debug_fn},
        {"command", viscmd_command_fn},
        {"exit", viscmd_exit_fn},
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
        {"emitnow", viscmd_emitnow_fn},
        {"get_debug", viscmd_get_debug_fn},
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
    NEW_CONST(BLEND_PARABOLIC);
    NEW_CONST(BLEND_QUADRATIC);
    NEW_CONST(BLEND_SINE);
    NEW_CONST(BLEND_NEGGAMMA);
    NEW_CONST(BLEND_EASING);
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
    s->dbg = DBMALLOC(sizeof(struct script_debug));
    s->fl = flist_new();
    s->L = luaL_newstate();
    luaL_openlibs(s->L);
    luaL_requiref(s->L, "Vis", initialize_vis_lib, 0);
    lua_pop(s->L, 1);

    /* adjust lua search path, include ./lua and ./test,
     * include default modules */
    script_run_string(s,
        "package = require('package')\n"
        "package.path = '; ;./?.lua;./lua/?.lua;./test/?.lua'"
        "Vis = require(\"Vis\")\n"
        "VisUtil = require(\"visutil\")\n"
        "Emit = require(\"emit\")\n"
        "Vis.on_mousedown = function() end\n"
        "Vis.on_mouseup = function() end\n"
        "Vis.on_mousemove = function() end\n"
        "Vis.on_keydown = function() end\n"
        "Vis.on_keyup = function() end\n"
        "Vis.on_quit = function() end\n"
    );
    VIS_ASSERT(s->errors == 0);
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

    lua_pop(s->L, 1); /* getglobal("Vis") */

    return s;
}

void script_free(script_t s) {
    if (!s) return;
    lua_close(s->L);
    DBFREE(s->dbg);
    DBFREE(s);
}

flist_t script_run(script_t script, const char* filename) {
    /* script->fl already bound to script in script_new */
    /* DBPRINTF("Running %s to build %p", filename, script->fl); */
    if (luaL_dofile(script->L, filename) != LUA_OK) {
        script->errors += 1;
        eprintf("Error in script %s: %s", filename,
                luautil_get_error(script->L));
    }
    return script->fl;
}

void script_run_string(script_t script, const char* torun) {
    if (luaL_dostring(script->L, torun) != LUA_OK) {
        script->errors += 1;
        char* esc = escape_string(torun);
        eprintf("Error in script \"%s\": %s", esc,
                luautil_get_error(script->L));
        free(esc);
    }
}

void script_run_cb(script_t script, script_cb_t cb, UNUSED_PARAM(void* args)) {
    script_run_string(script, cb->fn_code);
}

void script_set_drawer(script_t script, drawer_t drawer) {
    script->drawer = drawer;
}

void script_callback_free(script_cb_t cb) {
    DBFREE(cb->fn_name);
    DBFREE(cb->fn_code);
    DBFREE(cb);
}

int script_get_status(script_t script) {
    return script->errors;
}

void script_set_debug(script_t script, enum script_debug_id what, uint32_t n) {
    switch (what) {
        case SCRIPT_DEBUG_PARTICLES_EMITTED:
            script->dbg->particles_emitted = n;
            break;
        case SCRIPT_DEBUG_TIME_NOW:
            script->dbg->time_now = n;
            break;
        case SCRIPT_DEBUG_FRAMES_EMITTED:
            script->dbg->frames_emitted = n;
            break;
        case SCRIPT_DEBUG_NUM_MUTATES:
            script->dbg->num_mutates = n;
            break;
    }
}

void script_get_debug(script_t script, script_debug_t dbg) {
    *dbg = *script->dbg;
}

void script_mousemove(script_t script, int x, int y) {
    script->errors += do_mouse_event(script->L, "mousemove", x, y);
}

void script_mousedown(script_t script, int x, int y) {
    script->errors += do_mouse_event(script->L, "mousedown", x, y);
}

void script_mouseup(script_t script, int x, int y) {
    script->errors += do_mouse_event(script->L, "mouseup", x, y);
}

void script_keydown(script_t script, const char* keyname, BOOL shift) {
    script->errors += do_keyboard_event(script->L, "keydown", keyname, shift);
}

void script_keyup(script_t script, const char* keyname, BOOL shift) {
    script->errors += do_keyboard_event(script->L, "keyup", keyname, shift);
}

void script_on_quit(script_t script) {
    script_run_string(script, "Vis.on_quit()");
}

/* end of public API */
script_t luautil_checkscript(lua_State* L, int arg) {
    /* calling a disabled function is not an error */
    if (lua_isnil(L, arg)) {
        luaL_error(L, "expected Vis.script, received nil, "
                      "function may be disabled");
        return NULL;
    } else {
        return *(script_t*)luaL_checkudata(L, arg++, "script_t*");
    }
}

void luautil_checkdrawer(lua_State* L, script_t script) {
    if (script->drawer == NULL) {
        luaL_error(L, "Script has no drawer, function may be disabled");
    }
}

const char* luautil_get_error(lua_State* L) {
    static char error_buff[2048];
    strncpy(error_buff, luaL_checkstring(L, -1), 2048);
    lua_pop(L, 1);
    return error_buff;
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
    /* DBPRINTF("command(%p, %d, \"%s\")", fl, when, cmd); */
    flist_insert_cmd(fl, when, cmd);
    return 0;
}

/* Vis.exit(Vis.flist, when) */
int viscmd_exit_fn(lua_State* L) {
    flist_t fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    fnum_t when = (fnum_t)VIS_MSEC_TO_FRAMES(luaL_checkunsigned(L, 2));
    flist_insert_exit(fl, when);
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

/* Vis.audio(Vis.flist, 0, path),
 * @param path must resolve to a .WAV file */
int viscmd_audio_fn(lua_State* L) {
    flist_t fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    fnum_t when = (fnum_t)VIS_MSEC_TO_FRAMES(luaL_checkunsigned(L, 2));
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
    luautil_checkdrawer(L, s);
    double r = luaL_optnumber(L, 2, 0);
    double g = luaL_optnumber(L, 3, 0);
    double b = luaL_optnumber(L, 4, 0);
    drawer_bgcolor(s->drawer, (float)r, (float)g, (float)b);
    DBPRINTF("Vis.bgcolor(%p, %g, %g, %g)", s->drawer, r, g, b);
    return 0;
}

/* Vis.mutate(Vis.flist, when, func, factor),
 * @param func is a valid Vis.MUTATE_* */
int viscmd_mutate_fn(lua_State* L) {
    mutate_method_t method = DBMALLOC(sizeof(struct mutate_method));
    flist_t fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    fnum_t when = (fnum_t)VIS_MSEC_TO_FRAMES(luaL_checkunsigned(L, 2));
    mutate_id fnid = (mutate_id)luaL_checkint(L, 3);
    double factor = luaL_checknumber(L, 4);
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
    /* DBPRINTF("Vis.callback(%p, %d, %p, %s)", fl, when, s, scb->fn_code); */
    flist_insert_scriptcb(fl, when, scb);
    return 0;
}

/* fps = Vis.fps(Vis.script) */
int viscmd_fps_fn(lua_State* L) {
    script_t s = luautil_checkscript(L, 1);
    luautil_checkdrawer(L, s);
    lua_pushnumber(L, (lua_Number)drawer_get_fps(s->drawer));
    return 1;
}

/* Vis.settrace(Vis.script, ...)
 * Function has same args as Vis.emit except:
 *      @param Vis.flist is replaced with Vis.script
 *      @param when is omitted */
int viscmd_settrace_fn(lua_State* L) {
    script_t s = luautil_checkscript(L, 1);
    luautil_checkdrawer(L, s);
    drawer_set_trace(s->drawer, lua_args_to_emit_t(L, 2, NULL));
    return 0;
}

/* Vis.emitnow(Vis.script, ...)
 * Function has the same args as Vis.settrace */
int viscmd_emitnow_fn(lua_State* L) {
    /* Vis.script is actually ignored, but is present for consistency */
    emit_t emit = lua_args_to_emit_t(L, 2, NULL);
    emit_frame(emit);
    emit_free(emit);
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

/* Vis.get_debug(Vis.script, what) */
int viscmd_get_debug_fn(lua_State* L) {
    script_t s = luautil_checkscript(L, 1);
    const char* what = luaL_checkstring(L, 2);
    if (!strcmp(what, "PARTICLES-EMITTED")) {
        lua_pushunsigned(L, s->dbg->particles_emitted);
    } else if (!strcmp(what, "TIME-NOW")) {
        lua_pushunsigned(L, s->dbg->time_now);
    } else if (!strcmp(what, "FRAMES-EMITTED")) {
        lua_pushunsigned(L, s->dbg->frames_emitted);
    } else if (!strcmp(what, "NUM-MUTATES")) {
        lua_pushunsigned(L, s->dbg->num_mutates);
    } else {
        s->errors += 1;
        return luaL_error(L, "Debug token \"%s\" invalid", what);
    }
    return 1;
}

static int do_mouse_event(lua_State* L, const char* func, int x, int y) {
    kstr s = kstring_newfromvf("Vis.on_%s(%d, %d)", func, x, y);
    int nerror = 0;
    if (luaL_dostring(L, kstring_content(s)) != LUA_OK) {
        const char* error = luaL_checkstring(L, -1);
        eprintf("Error in %s: %s", kstring_content(s), error);
        lua_pop(L, 1);
        nerror = 1;
    }
    kstring_free(s);
    return nerror;
}

static int do_keyboard_event(lua_State* L, const char* func, const char* key,
                             BOOL shift) {
    int nerror = 0;
    char* esc_key = escape_string(key);
    kstr s = kstring_newfromvf("Vis.on_%s(\"%s\", %d)", func, esc_key, (int)shift);
    if (luaL_dostring(L, kstring_content(s)) != LUA_OK) {
        const char* error = luaL_checkstring(L, -1);
        eprintf("Error in %s: %s", kstring_content(s), error);
        lua_pop(L, 1);
        nerror = 1;
    }
    kstring_free(s);
    free(esc_key);
    return nerror;
}

static emit_t lua_args_to_emit_t(lua_State* L, int arg, fnum_t* when) {
    emit_t emit = emit_new();
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

