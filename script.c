
#include "audio.h"
#include "helper.h"
#include "script.h"

#include <string.h>
#include <stdarg.h>

#ifdef VIS_USE_LUA

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
        DBPRINTF("arg[%d] = ?", i);
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
    int color[3], ucolor[3];
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
    color[0] = luaL_optint(L, arg++, 255);
    color[1] = luaL_optint(L, arg++, 255);
    color[2] = luaL_optint(L, arg++, 255);
    color[3] = luaL_optint(L, arg++, 0);
    color[4] = luaL_optint(L, arg++, 0);
    color[5] = luaL_optint(L, arg++, 0);
    force = luaL_optint(L, arg++, 0);
    limit = luaL_optint(L, arg++, 0);
    blender = luaL_optint(L, arg++, 0);

    DBPRINTF("emit(%p, %d, %d, %g, %g, %g, %g, %g, %g, %g, %g, %g, %g, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", fl, n, when, x, y, ux, uy, radius, uradius, ds, uds, theta, utheta, life, ulife, color[0], color[1], color[2], color[3], color[4], color[5], force, limit, blender);

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

int viscmd_bgcolor_fn(lua_State* L) { return 0; }

int viscmd_mutate_fn(lua_State* L) { return 0; }

#else

#include "squirrel.h"
#include "sqstdio.h"
#include "sqstdmath.h"
#include "sqstdaux.h"
#include "sqstdstring.h"

#define VIS_SQUIRREL_MEM_LIMIT 1024

typedef SQInteger (*closure_t)(HSQUIRRELVM);

SQInteger print_args(HSQUIRRELVM v);

void sq_printfunc(UNUSED_PARAM(HSQUIRRELVM v), const SQChar *s, ...) {
    va_list vl;
    va_start(vl, s);
    vfprintf(stdout, s, vl);
    va_end(vl);
}

void sq_errorfunc(UNUSED_PARAM(HSQUIRRELVM v), const SQChar *s, ...) {
    va_list vl;
    va_start(vl, s);
    vfprintf(stderr, s, vl);
    va_end(vl);
}

int make_squirrel_vm(HSQUIRRELVM* sqvm) {
    *sqvm = sq_open(VIS_SQUIRREL_MEM_LIMIT);
    if (*sqvm == NULL) {
        eprintf("fatal: unable to open the Squirrel vm");
        return 2;
    }
    sq_pushroottable(*sqvm);
    sqstd_register_mathlib(*sqvm);
    sqstd_register_stringlib(*sqvm);
    DBPRINTF("registered the mathlib and stringlib", "");
    return 0;
}

void squirrel_add_global(HSQUIRRELVM sqvm, const char* varname, void* value) {
    sq_pushroottable(sqvm);
    sq_pushstring(sqvm, varname, -1);
    sq_pushuserpointer(sqvm, value);
    sq_newslot(sqvm, -3, SQFalse);
    sq_pop(sqvm, 1);
}

void squirrel_add_constant(HSQUIRRELVM sqvm, const char* varname, int value) {
    sq_pushroottable(sqvm);
    sq_pushstring(sqvm, varname, -1);
    sq_pushinteger(sqvm, value);
    sq_newslot(sqvm, -3, SQFalse);
    sq_pop(sqvm, 1);
}

void squirrel_add_closure(HSQUIRRELVM sqvm, const char* fnname, closure_t fn) {
    sq_pushroottable(sqvm);
    sq_pushstring(sqvm, fnname, -1);
    sq_newclosure(sqvm, fn, 0);
    sq_newslot(sqvm, -3, SQFalse);
    sq_pop(sqvm, 1);
}

SQInteger squirrel_emit_fn(HSQUIRRELVM sqvm);
SQInteger squirrel_audio_fn(HSQUIRRELVM sqvm);
SQInteger squirrel_bgcolor_fn(HSQUIRRELVM sqvm);
SQInteger squirrel_mutate_fn(HSQUIRRELVM sqvm);

flist_t load_script(const char* filename) {
    int result = 0;
    HSQUIRRELVM sqvm;
    flist_t fl = NULL;
    result = make_squirrel_vm(&sqvm);
    if (result == 1) { sq_close(sqvm); }
    if (result != 0) { return NULL; }
    sq_setprintfunc(sqvm, sq_printfunc, sq_errorfunc);
    sqstd_seterrorhandlers(sqvm);
    fl = flist_new();
    
    /* grant access to the flist */
    squirrel_add_global(sqvm, "__flist_instance__", fl);
#define NEW_CONST(handle) squirrel_add_constant(sqvm, #handle, handle)
    /* grant access to all of the enums and constants */
    NEW_CONST(VIS_FPS_LIMIT);
    NEW_CONST(VIS_WIDTH);
    NEW_CONST(VIS_HEIGHT);
    /* blenders */
    NEW_CONST(VIS_NO_BLEND);
    NEW_CONST(VIS_DEFAULT_BLEND);
    NEW_CONST(VIS_BLEND_LINEAR);
    NEW_CONST(VIS_BLEND_QUADRATIC);
    NEW_CONST(VIS_BLEND_NEGGAMMA);
    NEW_CONST(VIS_NBLENDS);
    /* forces */
    NEW_CONST(VIS_DEFAULT_FORCE);
    NEW_CONST(VIS_FRICTION);
    NEW_CONST(VIS_GRAVITY);
    NEW_CONST(VIS_NFORCES);
    /* limits */
    NEW_CONST(VIS_DEFAULT_LIMIT);
    NEW_CONST(VIS_BOX);
    NEW_CONST(VIS_SPRINGBOX);
    NEW_CONST(VIS_NLIMITS);
    /* mutators */
    NEW_CONST(VIS_MUTATE_PUSH);
    NEW_CONST(VIS_MUTATE_SLOW);
    NEW_CONST(VIS_MUTATE_SHRINK);
    NEW_CONST(VIS_MUTATE_GROW);
    NEW_CONST(VIS_NMUTATES);
#undef NEW_CONST
    /* push the following functions */
    squirrel_add_closure(sqvm, "emit", squirrel_emit_fn);
    squirrel_add_closure(sqvm, "audio", squirrel_audio_fn);
    squirrel_add_closure(sqvm, "bgcolor", squirrel_bgcolor_fn);
    squirrel_add_closure(sqvm, "mutate", squirrel_mutate_fn);
    
    sqstd_dofile(sqvm, filename, SQFalse, SQTrue);
    
    audio_play();
    flist_restart(fl);
    
    sq_close(sqvm);
    return fl;
}

SQInteger squirrel_emit_fn(HSQUIRRELVM sqvm) {
    SQInteger i;
    flist_t fl;
    frame_t frame;
    SQInteger nparticles, when;
    SQFloat where[4] = {0, 0, 0, 0};
    SQFloat size[2] = {0, 0};
    SQFloat speed[2] = {0, 0};
    SQFloat angle[2] = {0, 0};
    SQInteger life[2] = {0, 0};
    SQFloat color[6] = {0, 0, 0, 0, 0, 0};
    SQInteger force, limit, blender;
    SQInteger nargs = sq_gettop(sqvm);
    
    /* sqvm flist when n x y ... */
    /* n x y ux uy rad urad ds uds theta utheta life ulife r g b ur ug ub
       force limit blender */
    /* start at stack position 2 */
    i = 2;
    sq_getuserpointer(sqvm, i++, (void**)&fl);
    sq_getinteger(sqvm, i++, &when);
    sq_getinteger(sqvm, i++, &nparticles);
    sq_getfloat(sqvm, i++, where);
    sq_getfloat(sqvm, i++, where + 1);
    sq_getfloat(sqvm, i++, where + 2);
    sq_getfloat(sqvm, i++, where + 3);
    sq_getfloat(sqvm, i++, size);
    sq_getfloat(sqvm, i++, size + 1);
    sq_getfloat(sqvm, i++, speed);
    sq_getfloat(sqvm, i++, speed + 1);
    sq_getfloat(sqvm, i++, angle);
    sq_getfloat(sqvm, i++, angle + 1);
    sq_getinteger(sqvm, i++, life);
    sq_getinteger(sqvm, i++, life + 1);
    sq_getfloat(sqvm, i++, color);
    sq_getfloat(sqvm, i++, color + 1);
    sq_getfloat(sqvm, i++, color + 2);
    sq_getfloat(sqvm, i++, color + 3);
    sq_getfloat(sqvm, i++, color + 4);
    sq_getfloat(sqvm, i++, color + 5);
    sq_getinteger(sqvm, i++, &force);
    sq_getinteger(sqvm, i++, &limit);
    sq_getinteger(sqvm, i++, &blender);
    
    DBPRINTF("** Received a call to emit(nargs = %d) **", nargs);
    DBPRINTF("%d particles at (%g,%g)\xc2\xb1(%g,%g)", nparticles,
             where[0], where[1], where[2], where[3]);
    DBPRINTF("size %g\xc2\xb1%g, speed %g\xc2\xb1%g, angle %g\xc2\xb1%g",
             size[0], size[1],
             speed[0], speed[1], angle[0], angle[1]);
    DBPRINTF("life %d\xc2\xb1%d, color (%g, %g, %g)\xc2\xb1(%g, %g, %g)",
             life[0], life[1],
             color[0], color[1], color[2], color[3], color[4], color[5]);
    
    frame = make_emit_frame((int)nparticles,
        where[0], where[1], where[2], where[3],
        size[0], size[1], speed[0], speed[1], angle[0], angle[1],
        (int)life[0], (int)life[1],
        color[0], color[1], color[2], color[3], color[4], color[5],
        (force_t)force, (limit_t)limit, (blend_t)blender);
    
    flist_insert_emit(fl, (fnum_t)when, frame);
    
    return 0;
}

SQInteger squirrel_audio_fn(HSQUIRRELVM sqvm) {
    const SQChar* filename = NULL;
    SQInteger nargs = sq_gettop(sqvm);
    if (nargs == 2) {
        sq_getstring(sqvm, 2, &filename);
    }
    if (filename != NULL) {
        audio_open(filename);
    }
    return 0;
}

SQInteger squirrel_bgcolor_fn(HSQUIRRELVM sqvm) {
    SQInteger nargs = sq_gettop(sqvm);
    DBPRINTF("Received a call to bgcolor(nargs = %d)", nargs);
    /* args: 1:sqvm, 2:flist, 3:when, 4:r, 5:g, 6:b */
    if (nargs == 5) {
        flist_t fl;
        SQInteger when;
        SQFloat color[3];
        sq_getuserpointer(sqvm, 2, (void**)&fl);
        sq_getinteger(sqvm, 3, &when);
        sq_getfloat(sqvm, 4, color);
        sq_getfloat(sqvm, 5, color + 1);
        sq_getfloat(sqvm, 6, color + 2);
        flist_insert_bgcolor(fl, (fnum_t)when, color);
    }
    return 0;
}

SQInteger squirrel_mutate_fn(HSQUIRRELVM sqvm) {
    SQInteger nargs = sq_gettop(sqvm);
    DBPRINTF("Received a call to mutate(nargs = %d)", nargs);
    /* TODO: MUTATE */
    return 0;
}

SQInteger print_args(HSQUIRRELVM v) {
    SQInteger nargs = sq_gettop(v); //number of arguments
    for(SQInteger n=1;n<=nargs;n++) {
        printf("arg %d is ", (int)n);
        switch(sq_gettype(v, n)) {
            case OT_NULL:
                printf("null\n");
                break;
            case OT_INTEGER:
                printf("integer\n");
                break;
            case OT_FLOAT:
                printf("float\n");
                break;
            case OT_STRING:
                printf("string\n");
                break;
            case OT_TABLE:
                printf("table\n");
                break;
            case OT_ARRAY:
                printf("array\n");
                break;
            case OT_USERDATA:
                printf("userdata\n");
                break;
            case OT_CLOSURE:
                printf("closure(function)\n");
                break;
            case OT_NATIVECLOSURE:
                printf("native closure(C function)\n");
                break;
            case OT_GENERATOR:
                printf("generator\n");
                break;
            case OT_USERPOINTER:
                printf("userpointer\n");
                break;
            case OT_CLASS:
                printf("class\n");
                break;
            case OT_INSTANCE:
                printf("instance\n");
                break;
            case OT_WEAKREF:
                printf("weak reference\n");
                break;
            case OT_BOOL:
                printf("boolean\n");
                break;
            case OT_THREAD:
                printf("thread\n");
                break;
            case OT_FUNCPROTO:
                printf("function prototype\n");
                break;
            case OT_OUTER:
                printf("outer\n");
                break;
            default:
                return sq_throwerror(v, "invalid param"); //throws an exception
        }
    }
    printf("\n");
    return 0;
}

#endif // VIS_USE_LUA

