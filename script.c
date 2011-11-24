
#include "audio.h"
#include "helper.h"
#include "script.h"

#include "squirrel.h"
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

SQRESULT make_squirrel_vm(HSQUIRRELVM* sqvm) {
  SQRESULT result = 0;
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

flist_t load_script(const char* filename) {
  SQRESULT result = 0;
  HSQUIRRELVM sqvm;
  flist_t fl = NULL;
  result = make_squirrel_vm(&sqvm);
	sq_setprintfunc(sqvm, sq_printfunc, sq_errorfunc);
  sq_pushroottable(sqvm);
  if (result == 1) { sq_close(sqvm); }
  if (result != 0) { return NULL; }
  fl = flist_new();
  
  sq_close(sqvm);
  return fl;
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

