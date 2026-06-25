
#ifndef VIS_CLARGS_HEADER_INCLUDED_
#define VIS_CLARGS_HEADER_INCLUDED_ 1

#include "defines.h"
#include "klist.h"
#include <stdint.h>

#ifdef HAVE_JSONC
#include <json.h>
#endif

struct clargs {
    const char* configfile;
    const char* execname;
    klist scriptfiles;
    klist scriptstrings;
    const char* dumpfile;
    int frameskip;
    int frames_per_second;
    klist scriptargs;
    BOOL dumptrace;
    BOOL interactive;
    BOOL quiet_audio;
    BOOL stay_after_script;
    BOOL absolute_fps;
    unsigned int wsize[2];

    int debug_level;

    BOOL must_exit;
    int exit_status;
#ifdef HAVE_JSONC
    json_object* configobj;
#endif
};
typedef struct clargs* clargs_t;

clargs_t argparse(int argc, char** argv);
void clargs_free(clargs_t args);

/* Functions for extracting data from configobj */
BOOL clargs_config_has(const clargs_t args, const char* key);
const char* clargs_config_get(const clargs_t args, const char* key);
int clargs_config_get_int(const clargs_t args, const char* key);
int clargs_config_get_bool(const clargs_t args, const char* key);
int64_t clargs_config_get_int64(const clargs_t args, const char* key);
double clargs_config_get_double(const clargs_t args, const char* key);

#endif
