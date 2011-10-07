
#include "audio.h"
#include "helper.h"
#include "script.h"

#include <string.h>

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

