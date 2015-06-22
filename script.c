
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

struct script {
    lua_State* L;
    script_cb_t callback;
    flist_t fl;
};

int luaopen_Vis(lua_State* L) {
    DBPRINTF("Loading lualibVis %p", L);
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
    NEW_CONST(NO_BLEND);
    NEW_CONST(DEFAULT_BLEND);
    NEW_CONST(BLEND_LINEAR);
    NEW_CONST(BLEND_QUADRATIC);
    NEW_CONST(BLEND_NEGGAMMA);
    NEW_CONST(NBLENDS);
    /* forces */
    NEW_CONST(DEFAULT_FORCE);
    NEW_CONST(FRICTION);
    NEW_CONST(GRAVITY);
    NEW_CONST(NFORCES);
    /* limits */
    NEW_CONST(DEFAULT_LIMIT);
    NEW_CONST(BOX);
    NEW_CONST(SPRINGBOX);
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

script_t script_new(void) {
    script_t s = chmalloc(sizeof(struct script));
    s->fl = flist_new();
    s->L = luaL_newstate();
    luaL_openlibs(s->L);
    luaL_requiref(s->L, "Vis", luaopen_Vis, 0);

    (void)luaL_dostring(s->L, "Vis = require(\"Vis\")");

    flist_t* flbox = lua_newuserdata(s->L, sizeof(void*));
    luaL_newmetatable(s->L, "flist_t*");
    lua_pop(s->L, 1);
    luaL_setmetatable(s->L, "flist_t*");
    *flbox = s->fl;
    lua_setfield(s->L, -2, "flist");
    script_t* sbox = lua_newuserdata(s->L, sizeof(void*));
    luaL_newmetatable(s->L, "script_t*");
    lua_pop(s->L, 1);
    luaL_setmetatable(s->L, "script_t*");
    *sbox = s;
    lua_setfield(s->L, -2, "script");
    lua_pop(s->L, 1);

    return s;
}

void script_destroy(script_t s) {
    if (!s) return;
    lua_close(s->L);
    free(s);
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

int viscmd_debug_fn(lua_State* L) {
    int nargs = 0, i = 0;
    flist_t fl = NULL;
    nargs = lua_gettop(L);
    DBPRINTF("Debug: L=%p", L);
    DBPRINTF("nargs=%d", nargs);
    fl = *(flist_t *)luaL_checkudata(L, 1, "flist_t*");
    DBPRINTF("arg[%d] = %p", 1, fl);
    for (i = 1; i <= nargs; ++i) {
        DBPRINTF("arg[%d] = %s", luaL_typename(L, i));
    }
    return 0;
}

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

int viscmd_emit_fn(lua_State* L) {
    flist_t fl = NULL;
    frame_t frame;
    int arg;
    int n, when;
    lua_Number x, y, ux, uy;
    lua_Number radius, uradius;
    lua_Number ds, uds;
    lua_Number theta, utheta;
    int life, ulife;
    float color[3], ucolor[3];
    int force, limit, blender;

    arg = 1;
    fl = *(flist_t*)luaL_checkudata(L, arg++, "flist_t*");
    n = luaL_checkint(L, arg++);
    when = luaL_checkint(L, arg++);
    x = luaL_checknumber(L, arg++);
    y = luaL_checknumber(L, arg++);
    ux = luaL_optnumber(L, arg++, 0);
    uy = luaL_optnumber(L, arg++, 0);
    radius = luaL_optnumber(L, arg++, 1);
    uradius = luaL_optnumber(L, arg++, 0);
    ds = luaL_optnumber(L, arg++, 0);
    uds = luaL_optnumber(L, arg++, 0);
    theta = luaL_optnumber(L, arg++, 0);
    utheta = luaL_optnumber(L, arg++, 0);
    life = luaL_optint(L, arg++, 100);
    ulife = luaL_optint(L, arg++, 0);
    color[0] = (float)luaL_optnumber(L, arg++, 1.0);
    color[1] = (float)luaL_optnumber(L, arg++, 1.0);
    color[2] = (float)luaL_optnumber(L, arg++, 1.0);
    color[3] = (float)luaL_optnumber(L, arg++, 0.0);
    color[4] = (float)luaL_optnumber(L, arg++, 0.0);
    color[5] = (float)luaL_optnumber(L, arg++, 0.0);
    force = luaL_optint(L, arg++, 0);
    limit = luaL_optint(L, arg++, 0);
    blender = luaL_optint(L, arg++, 0);

    DBPRINTF("emit(%p, %d, %d, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %d, %d, %g, %g, %g, %g, %g, %g, %d, %d, %d)", fl, n, when, x, y, ux, uy, radius, uradius, ds, uds, theta, utheta, life, ulife, color[0], color[1], color[2], color[3], color[4], color[5], force, limit, blender);

    frame = make_emit_frame(n, x, y, ux, uy, radius, uradius, ds, uds, theta, utheta, life, ulife, color[0], color[1], color[2], color[3], color[4], color[5], (force_t)force, (limit_t)limit, (blend_t)blender);
    flist_insert_emit(fl, (fnum_t)when, frame);

    return 0;
}

int viscmd_audio_fn(lua_State* L) {
    if (!audio_open(luaL_checkstring(L, 1))) {
        lua_pushnil(L);
    } else {
        audio_play();
        lua_pushboolean(L, TRUE);
    }
    return 1;
}

int viscmd_play_fn(UNUSED_PARAM(lua_State* L)) {
    audio_play();
    return 0;
}

int viscmd_pause_fn(UNUSED_PARAM(lua_State* L)) {
    audio_pause();
    return 0;
}

int viscmd_seek_fn(lua_State* L) {
    unsigned offset = luaL_checkunsigned(L, 1);
    audio_seek(offset);
    return 0;
}

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

int viscmd_bgcolor_fn(lua_State* L) {
    float r, g, b;
    r = (float)luaL_optnumber(L, 1, 0);
    g = (float)luaL_optnumber(L, 2, 0);
    b = (float)luaL_optnumber(L, 3, 0);
    set_background_color(r, g, b, 1);
    return 0;
}

int viscmd_mutate_fn(lua_State* L) {
    int nargs = lua_gettop(L);
    /* Vis.mutate(flist, when, func, factor) */
    flist_t fl = NULL;
    fnum_t when = NULL;
    mutate_t fnid = VIS_MUTATE_PUSH;
    double factor = 0;
    mutate_method_t method = chmalloc(sizeof(struct mutate_method));
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

int viscmd_callback_fn(lua_State* L) {
    flist_t fl = NULL;
    fnum_t when = 0;
    script_cb_t scb = NULL;
    script_t s = NULL;
    fl = *(flist_t*)luaL_checkudata(L, 1, "flist_t*");
    when = (fnum_t)luaL_checkint(L, 2);
    s = *(script_t*)luaL_checkudata(L, 3, "script_t*");
    scb = chmalloc(sizeof(struct script_cb));
    scb->owner = s;
    scb->fn_name = dupstr("<lua>");
    scb->fn_code = escape_string(luaL_checkstring(L, 4));
    flist_insert_scriptcb(fl, when, scb);
    return 0;
}

