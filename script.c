
#include "audio.h"
#include "helper.h"
#include "script.h"

#include "squirrel.h"
#include "sqstdio.h"
#include "sqstdmath.h"
#include "sqstdaux.h"
#include "sqstdstring.h"

#include <string.h>
#include <stdarg.h>

#define VIS_SQUIRREL_MEM_LIMIT 1024

#ifdef SQUNICODE
#define scfprintf fwprintf
#define scfopen	_wfopen
#define scvprintf vfwprintf
#else
#define scfprintf fprintf
#define scfopen	fopen
#define scvprintf vfprintf
#endif

typedef SQInteger (*closure_t)(HSQUIRRELVM);

SQInteger print_args(HSQUIRRELVM v);

void sq_printfunc(UNUSED_PARAM(HSQUIRRELVM v), const SQChar *s, ...) {
	va_list vl;
	va_start(vl, s);
	scvprintf(stdout, s, vl);
	va_end(vl);
}

void sq_errorfunc(UNUSED_PARAM(HSQUIRRELVM v), const SQChar *s, ...) {
	va_list vl;
	va_start(vl, s);
	scvprintf(stderr, s, vl);
	va_end(vl);
}

int make_squirrel_vm(HSQUIRRELVM* sqvm) {
  int result = 0;
  *sqvm = sq_open(VIS_SQUIRREL_MEM_LIMIT);
  if (*sqvm == NULL) {
    eprintf("fatal: unable to open the Squirrel vm");
    return 2;
  }
  if (!SQ_SUCCEEDED(sqstd_register_mathlib(*sqvm))) {
    eprintf("fatal: unable to register the Squirrel math library");
    result = 1;
  }
  if (!SQ_SUCCEEDED(sqstd_register_stringlib(*sqvm))) {
    eprintf("fatal: unable to register the Squirrel string library");
    result = 1;
  }
  return result;
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
  sq_pushroottable(sqvm);
  fl = flist_new();
  
  /* grant access to the flist */
  squirrel_add_global(sqvm, "__flist_instance__", fl);
#define NEW_GLOBAL(handle) squirrel_add_constant(sqvm, #handle, VIS_##handle)
  /* grant access to all of the enums and constants */
  NEW_GLOBAL(FPS_LIMIT);
  NEW_GLOBAL(WIDTH);
  NEW_GLOBAL(HEIGHT);
  /* blenders */
  NEW_GLOBAL(NO_BLEND);
  NEW_GLOBAL(DEFAULT_BLEND);
  NEW_GLOBAL(BLEND_LINEAR);
  NEW_GLOBAL(BLEND_QUADRATIC);
  NEW_GLOBAL(BLEND_NEGGAMMA);
  NEW_GLOBAL(NBLENDS);
  /* forces */
  NEW_GLOBAL(DEFAULT_FORCE);
  NEW_GLOBAL(FRICTION);
  NEW_GLOBAL(GRAVITY);
  NEW_GLOBAL(NFORCES);
  /* limits */
  NEW_GLOBAL(DEFAULT_LIMIT);
  NEW_GLOBAL(BOX);
  NEW_GLOBAL(SPRINGBOX);
  NEW_GLOBAL(NLIMITS);
  /* mutators */
  NEW_GLOBAL(MUTATE_PUSH);
  NEW_GLOBAL(MUTATE_SLOW);
  NEW_GLOBAL(MUTATE_SHRINK);
  NEW_GLOBAL(MUTATE_GROW);
  NEW_GLOBAL(NMUTATES);
#undef NEW_GLOBAL
  /* push the following functions */
  squirrel_add_closure(sqvm, "emit", squirrel_emit_fn);
  squirrel_add_closure(sqvm, "audio", squirrel_audio_fn);
  squirrel_add_closure(sqvm, "bgcolor", squirrel_bgcolor_fn);
  squirrel_add_closure(sqvm, "mutate", squirrel_mutate_fn);
  
  sqstd_dofile(sqvm, filename, SQFalse, SQTrue);
  
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
  SQFloat velocity[4] = {0, 0, 0, 0};
  SQInteger life[2] = {0, 0};
  SQFloat color[6] = {0, 0, 0, 0, 0, 0};
  SQInteger force, limit, blender;
  SQInteger nargs = sq_gettop(sqvm);
  DBPRINTF("** Received a call to emit(nargs = %d) **", nargs);
  
  /* sqvm flist when n x y ... */
  /* n x y ux uy rad urad ds uds theta utheta life ulife r g b ur ug ub force limit blender */
  /* start at stack position 2 */
  i = 2;
  sq_getuserpointer(sqvm, i++, (void**)&fl);
  sq_getinteger(sqvm, i++, &when);
  sq_getinteger(sqvm, i++, &nparticles);
  sq_getfloat(sqvm, i++, &where[0]);
  sq_getfloat(sqvm, i++, &where[1]);
  sq_getfloat(sqvm, i++, &where[2]);
  sq_getfloat(sqvm, i++, &where[3]);
  sq_getfloat(sqvm, i++, size);
  sq_getfloat(sqvm, i++, size + 1);
  sq_getfloat(sqvm, i++, velocity);
  sq_getfloat(sqvm, i++, velocity + 1);
  sq_getfloat(sqvm, i++, velocity + 2);
  sq_getfloat(sqvm, i++, velocity + 3);
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
  
  DBPRINTF("%d particles at (%g,%g)\xc2\xb1(%g,%g)", nparticles, where[0], where[1], where[2], where[3]);
  DBPRINTF("size %g\xc2\xb1%g, velocity |%g,%g>\xc2\xb1|%g,%g>", size[0], size[1], velocity[0], velocity[2], velocity[1], velocity[3]);
  DBPRINTF("life %d\xc2\xb1%d, color (%g, %g, %g)\xc2\xb1(%g, %g, %g)", life[0], life[1], color[0], color[1], color[2], color[3], color[4], color[5]);
  
  frame = make_emit_frame(nparticles, where[0], where[1], where[2], where[3],
    size[0], size[1], velocity[0], velocity[1], velocity[2], velocity[3],
    life[0], life[1],
    color[0], color[1], color[2], color[3], color[4], color[5],
    force, limit, blender);
  
  flist_insert_emit(fl, when, frame);
  
  return 0;
}

SQInteger squirrel_audio_fn(HSQUIRRELVM sqvm) {
  SQInteger nargs = sq_gettop(sqvm);
  DBPRINTF("Received a call to audio(nargs = %d): %p", nargs, sqvm);
  return 0;
}

SQInteger squirrel_bgcolor_fn(HSQUIRRELVM sqvm) {
  SQInteger nargs = sq_gettop(sqvm);
  DBPRINTF("Received a call to bgcolor(nargs = %d): %p", nargs, sqvm);
  return 0;
}

SQInteger squirrel_mutate_fn(HSQUIRRELVM sqvm) {
  SQInteger nargs = sq_gettop(sqvm);
  DBPRINTF("Received a call to mutate(nargs = %d): %p", nargs, sqvm);
  return 0;
}

#if 0
void parse_line(flist_t fl, const char* line, unsigned lnum);

flist_t load_script(const char* filename) {
  flist_t fl = flist_new();
  FILE* script = NULL;
  char line[VIS_BUFFER_LEN];
  unsigned lnum = 0;
  
  script = try_fopen(filename, "r");
  if (script == NULL) { return NULL; }
  
  while (!ferror(script) && !feof(script)) {
    ++lnum;
    if (fgets(line, VIS_BUFFER_LEN, script) == NULL) {
      if (ferror(script) || feof(script)) {
        break;
      } else {
        eprintf("%d: bad input line", lnum);
        continue;
      }
    }
    if (line[0] == '#' || line[0] == '\n') {
      continue;
    }
    *strnulchr(line, '\n') = '\0';
    parse_line(fl, line, lnum);
  }
  audio_play();
  flist_restart(fl);
  
  fclose(script);
  return fl;
}

void parse_line(flist_t fl, const char* line, unsigned lnum) {
  unsigned when;
  const char* cmd;
  if (startswith(line, "audio ")) {
    audio_open(line + strlen("audio "));
  } else if (startswith(line, "seek ")) {
    unsigned int arg1, arg2, arg3;
    if (sscanf(line, "seek %u:%u.%u", &arg1, &arg2, &arg3) == 3) {
      audio_seek((arg1 * 60 + arg2) * 100 + arg3);
    }
  } else {
    unsigned nframe = 0;
    frame_t frame = NULL;
    /* <frame> <command> [args...] */
    if (sscanf(line, "%u ", &when) != 1) {
      eprintf("%d: bad input line", lnum);
      return;
    }
    /* frame = when * FPS / csec-per-sec */
    nframe = when * VIS_FPS_LIMIT / 100;
    if (nframe >= VIS_NFRAMES) {
      eprintf("%d: too large of an offset %d", lnum, when);
      return;
    }
    /* needs a space between time and command */
    cmd = strchr(line, ' ');
    if (*cmd != ' ') {
      eprintf("%d: bad input line", lnum);
      return;
    }
    /* skip past the space */
    ++cmd;
    if (startswith(cmd, "exit")) {
      flist_insert_cmd(fl, nframe, "exit");
    } else if (startswith(cmd, "kick ")) {
      frame = make_kick_frame(strtoi(cmd + strlen("kick ")));
      flist_insert_emit(fl, nframe, frame);
    } else if (startswith(cmd, "strum ")) {
      frame = make_strum_frame(strtoi(cmd + strlen("strum ")));
      flist_insert_emit(fl, nframe, frame);
    } else if (startswith(cmd, "snare ")) {
      frame = make_snare_frame(strtoi(cmd + strlen("snare ")));
      flist_insert_emit(fl, nframe, frame);
    } else if (startswith(cmd, "rain ")) {
      frame = make_rain_frame(strtoi(cmd + strlen("rain ")));
      flist_insert_emit(fl, nframe, frame);
    } else if (startswith(cmd, "emit ")) {
      /* 22 arguments total */
      static const int nargs = 22;
      int n;
      double x, y, ux, uy, radius, uradius, ds, uds, theta, utheta;
      int life, ulife;
      float r, g, b, ur, ug, ub;
      unsigned force, limit, blender;
      if (sscanf(cmd, "emit %d %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %d "
                      "%d %g %g %g %g %g %g %u %u %u",
                 &n, &x, &y, &ux, &uy, &radius, &uradius, &ds, &uds, &theta,
                 &utheta, &life, &ulife, &r, &g, &b, &ur, &ug, &ub, &force,
                 &limit, &blender) == nargs) {
        
        if (force >= VIS_NFORCES) {
          force = VIS_DEFAULT_FORCE;
        }
        if (limit >= VIS_NLIMITS) {
          limit = VIS_DEFAULT_LIMIT;
        }
        if (blender >= VIS_NBLENDS) {
          blender = VIS_DEFAULT_BLEND;
        }
        
        frame = make_emit_frame(n, x, y, ux, uy,
                                radius, uradius, life, ulife,
                                ds, uds, theta, utheta,
                                r, g, b, ur, ug, ub,
                                force, limit, blender);
        flist_insert_emit(fl, nframe, frame);
      }
    } else {
      eprintf("%d: unknown command: %s", lnum, cmd);
    }
  }
}
#endif

SQInteger print_args(HSQUIRRELVM v)
{
  SQInteger nargs = sq_gettop(v); //number of arguments
  for(SQInteger n=1;n<=nargs;n++)
  {
    printf("arg %d is ",n);
    switch(sq_gettype(v,n))
    {
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
      default:
        return sq_throwerror(v, "invalid param"); //throws an exception
    }
  }
  printf("\n");
  return 0;
}


