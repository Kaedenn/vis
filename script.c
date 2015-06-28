
#include "audio.h"
#include "helper.h"
#include "flist.h"
#include "script.h"
#include "mutator.h"

#include <string.h>
#include <stdarg.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int viscmd_debug_fn(lua_State* L);

int viscmd_command_fn(lua_State* L);
int viscmd_emit_fn(lua_State* L);
int viscmd_audio_fn(lua_State* L);
int viscmd_play_fn(lua_State* L);
int viscmd_pause_fn(lua_State* L);
int viscmd_seek_fn(lua_State* L);
int viscmd_seekframe_fn(lua_State* L);
int viscmd_bgcolor_fn(lua_State* L);
int viscmd_mutate_fn(lua_State* L);
int viscmd_callback_fn(lua_State* L);
int viscmd_fps_fn(lua_State* L);
int viscmd_settrace_fn(lua_State* L);

static emit_t lua_args_to_emit_t(lua_State* L, int arg, fnum_t* when);
static void stack_dump(lua_State* L);

struct script {
    lua_State* L;
    flist_t fl;
    drawer_t drawer;
};

int luaopen_Vis(lua_State* L) {
    static const struct luaL_Reg vis_lib[] = {
        {"debug", viscmd_debug_fn},
        {"command", viscmd_command_fn},
        {"emit", viscmd_emit_fn},
        {"audio", viscmd_audio_fn},
        {"play", viscmd_play_fn},
        {"pause", viscmd_pause_fn},
        {"seek", viscmd_seek_fn},
        {"seekframe", viscmd_seekframe_fn},
        {"bgcolor", viscmd_bgcolor_fn},
        {"mutate", viscmd_mutate_fn},
        {"callback", viscmd_callback_fn},
        {"fps", viscmd_fps_fn},
        {"settrace", viscmd_settrace_fn},
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
    NEW_CONST(NMUTATES);
#undef NEW_CONST

    return 1;
}

script_t script_new(script_cfg_t cfg) {
    script_t s = DBMALLOC(sizeof(struct script));
    s->fl = flist_new();
    s->L = luaL_newstate();
    luaL_openlibs(s->L);
    luaL_requiref(s->L, "Vis", luaopen_Vis, 0);

    (void)luaL_dostring(s->L, "Vis = require(\"Vis\")");
    stack_dump(s->L);

    flist_t* flbox = lua_newuserdata(s->L, sizeof(void*));
    luaL_newmetatable(s->L, "flist_t*");
    lua_pop(s->L, 1);
    luaL_setmetatable(s->L, "flist_t*");
    *flbox = s->fl;
    lua_setfield(s->L, -2, "flist");
    if ((cfg & SCRIPT_NO_CB) == 0) {
        script_t* sbox = lua_newuserdata(s->L, sizeof(void*));
        luaL_newmetatable(s->L, "script_t*");
        lua_pop(s->L, 1);
        luaL_setmetatable(s->L, "script_t*");
        *sbox = s;
        lua_setfield(s->L, -2, "script");
        lua_pop(s->L, 1);
    }

    return s;
}

void script_free(script_t s) {
    if (!s) return;
    lua_close(s->L);
    DBFREE(s);
}

flist_t script_run(script_t s, const char* filename) {
    DBPRINTF("Running %s to build %p", filename, s->fl);
    if (luaL_dofile(s->L, filename) != LUA_OK) {
        eprintf("Error: %s", lua_tostring(s->L, -1));
    }
    return s->fl;
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

/* Vis.debug(Vis.flist, ...) */
int viscmd_debug_fn(lua_State* L) {
    int nargs = 0, i = 0;
    flist_t fl = NULL;
    nargs = lua_gettop(L);
    DBPRINTF("Debug: L=%p", L);
    DBPRINTF("nargs=%d", nargs);
    fl = *(flist_t *)luaL_checkudata(L, 1, "flist_t*");
    DBPRINTF("arg[%d] = (flist_t)%p", 1, fl);
    for (i = 1; i <= nargs; ++i) {
        int type = lua_type(L, i);
        DBPRINTF("arg[%d] = %s (%d)", i, lua_typename(L, type), type);
    }
    return 0;
}

/* Vis.command(Vis.flist, when, "command") */
int viscmd_command_fn(lua_State* L) {
    flist_t fl = NULL;
    fnum_t when = 0;
    const char* cmd = NULL;
    fl = *(flist_t *)luaL_checkudata(L, 1, "flist_t*");
    when = (fnum_t)luaL_checkint(L, 2);
    cmd = luaL_checkstring(L, 3);
    DBPRINTF("command(%p, %d, \"%s\")", fl, when, cmd);
    flist_insert_cmd(fl, when, cmd);
    return 0;
}

/* Vis.emit(Vis.flist, n, when, x, y, [ux, uy, radius, uradius, ds, uds,
 *          theta, utheta, r, g, b, ur, ug, ub, force, limit, blender]),
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
    flist_t fl = NULL;
    emit_t frame;
    int arg = 1;
    fnum_t when;
    fl = *(flist_t*)luaL_checkudata(L, arg++, "flist_t*");
    frame = lua_args_to_emit_t(L, arg, &when);
    flist_insert_emit(fl, when, frame);
    return 0;
}

/* Vis.audio(path),
 * @param path must resolve to a .WAV file */
int viscmd_audio_fn(lua_State* L) {
    if (!audio_open(luaL_checkstring(L, 1))) {
        lua_pushnil(L);
    } else {
        audio_play();
        lua_pushboolean(L, TRUE);
    }
    return 1;
}

/* Vis.play() */
int viscmd_play_fn(UNUSED_PARAM(lua_State* L)) {
    audio_play();
    return 0;
}

/* Vis.pause() */
int viscmd_pause_fn(UNUSED_PARAM(lua_State* L)) {
    audio_pause();
    return 0;
}

/* Vis.seek(offset),
 * @param offset is in milliseconds */
int viscmd_seek_fn(lua_State* L) {
    unsigned offset = luaL_checkunsigned(L, 1);
    audio_seek(offset);
    return 0;
}

/* Vis.seekframe(Vis.flist, when, whereto) */
int viscmd_seekframe_fn(lua_State* L) {
    flist_t fl = NULL;
    fnum_t where = 0;
    fnum_t whereto = 0;
    fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    where = luaL_checkunsigned(L, 2);
    whereto = luaL_checkunsigned(L, 3);
    flist_insert_seekframe(fl, where, whereto);
    return 0;
}

/* Vis.bgcolor(r, g, b),
 * 0 <= @param rgb <= 1 */
int viscmd_bgcolor_fn(lua_State* L) {
    float r, g, b;
    r = (float)luaL_optnumber(L, 1, 0);
    g = (float)luaL_optnumber(L, 2, 0);
    b = (float)luaL_optnumber(L, 3, 0);
    set_background_color(r, g, b, 1);
    return 0;
}

/* Vis.mutate(Vis.flist, when, func, factor),
 * @param func is a valid Vis.MUTATE_* */
int viscmd_mutate_fn(lua_State* L) {
    int nargs = lua_gettop(L);
    flist_t fl = NULL;
    fnum_t when = NULL;
    mutate_t fnid = VIS_MUTATE_PUSH;
    double factor = 0;
    mutate_method_t method = DBMALLOC(sizeof(struct mutate_method));
    fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    when = (fnum_t)luaL_checkint(L, 2);
    fnid = (mutate_t)luaL_checkint(L, 3);
    factor = (double)luaL_checknumber(L, 4);
    if (fnid < (mutate_t)0) fnid = (mutate_t)0;
    if (fnid >= VIS_NMUTATES) fnid = (mutate_t)0;
    method->func = MUTATE_MAP[fnid];
    method->factor = factor;
    flist_insert_mutate(fl, when, method);
    return 0;
}

/* Vis.callback(Vis.flist, when, Vis.script, "code") */
int viscmd_callback_fn(lua_State* L) {
    flist_t fl = NULL;
    fnum_t when = 0;
    script_cb_t scb = NULL;
    script_t s = NULL;
    fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    when = (fnum_t)luaL_checkint(L, 2);
    s = *(script_t*)luaL_checkudata(L, 3, "script_t*");
    scb = DBMALLOC(sizeof(struct script_cb));
    scb->owner = s;
    scb->fn_name = dupstr("<lua>");
    scb->fn_code = escape_string(luaL_checkstring(L, 4));
    flist_insert_scriptcb(fl, when, scb);
    return 0;
}

/* fps = Vis.fps(Vis.script) */
int viscmd_fps_fn(lua_State* L) {
    script_t s = NULL;
    s = *(script_t*)luaL_checkudata(L, 1, "script_t*");
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
    script_t s = NULL;
    emit_t emit = NULL;
    s = *(script_t*)luaL_checkudata(L, arg++, "script_t*");
    if (s->drawer == NULL) {
        return luaL_error(L, "drawer is NULL; function is disabled");
    } else {
        drawer_set_emit(s->drawer, lua_args_to_emit_t(L, arg, NULL));
    }
    return 0;
}

static emit_t lua_args_to_emit_t(lua_State* L, int arg, fnum_t* when) {
    emit_t emit = DBMALLOC(sizeof(struct emit_frame));
    emit->n = luaL_checkint(L, arg++);
    if (when != NULL) {
        *when = (fnum_t)luaL_checkint(L, arg++);
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
    emit->life = luaL_optint(L, arg++, 100);
    emit->ulife = luaL_optint(L, arg++, 0);
    emit->r = (float)luaL_optnumber(L, arg++, 1.0);
    emit->g = (float)luaL_optnumber(L, arg++, 1.0);
    emit->b = (float)luaL_optnumber(L, arg++, 1.0);
    emit->ur = (float)luaL_optnumber(L, arg++, 0.0);
    emit->ug = (float)luaL_optnumber(L, arg++, 0.0);
    emit->ub = (float)luaL_optnumber(L, arg++, 0.0);
    emit->force = luaL_optint(L, arg++, 0);
    emit->limit = luaL_optint(L, arg++, 0);
    emit->blender = luaL_optint(L, arg++, 0);
    return emit;
}

static void stack_dump(lua_State* L) {
    int nelems = lua_gettop(L);
    DBPRINTF("Lua stack: %d item%s", nelems, nelems==1?"":"s");
}

