
#include "audio.h"
#include "helper.h"
#include "script.h"

#include <string.h>
#include <stdarg.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int viscmd_debug_fn(lua_State* L);

int viscmd_command_fn(lua_State* L);
int viscmd_emit_fn(lua_State* L);
int viscmd_audio_fn(lua_State* L);
int viscmd_bgcolor_fn(lua_State* L);
int viscmd_mutate_fn(lua_State* L);

int luaopen_Vis(lua_State* L) {
    DBPRINTF("Loading lualibVis %p", L);
    static const struct luaL_Reg vis_lib[] = {
        {"debug", viscmd_debug_fn},
        {"command", viscmd_command_fn},
        {"emit", viscmd_emit_fn},
        {"audio", viscmd_audio_fn},
        {"bgcolor", viscmd_bgcolor_fn},
        {"mutate", viscmd_mutate_fn},
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
    NEW_CONST(NMUTATES);
#undef NEW_CONST

    return 1;
}

lua_State* script_create_vm(flist_t fl) {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "Vis", luaopen_Vis, 0);

    flist_t* box = lua_newuserdata(L, sizeof(void*));
    luaL_newmetatable(L, "flist_t*");
    lua_pop(L, 1);
    luaL_setmetatable(L, "flist_t*");
    *box = fl;
    lua_setfield(L, -2, "flist");

    lua_pop(L, 1);
    return L;
}

flist_t load_script(const char* filename) {
    flist_t fl = flist_new();
    lua_State* L = script_create_vm(fl);
    DBPRINTF("Running %s to build %p", filename, fl);
    if (luaL_dofile(L, filename)) {
        eprintf("Error: %s", lua_tostring(L, -1));
    }
    lua_close(L);
    return fl;
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
        lua_pushboolean(L, TRUE);
    }
    return 1;
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
    DBPRINTF("mutate: %d args", nargs);
    return 0;
}


