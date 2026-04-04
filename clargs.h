
#ifndef VIS_CLARGS_HEADER_INCLUDED_
#define VIS_CLARGS_HEADER_INCLUDED_ 1

#include "defines.h"
#include "klist.h"

#ifdef HAVE_JSONC
#include <json.h>
#endif

typedef struct clargs {
    const char* configfile;
    const char* execname;
    const char* scriptfile;
    const char* scriptstring;
    const char* dumpfile;
    const char* commandfile;
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
} clargs;
typedef clargs* clargs_t;

clargs* argparse(int argc, char** argv);
void clargs_free(clargs* args);

/* Functions for extracting data from configobj */
const char* clargs_config_get(clargs* args, const char* key);
int clargs_config_geti(clargs* args, const char* key);
long clargs_config_getl(clargs* args, const char* key);
double clargs_config_getd(clargs* args, const char* key);

#endif
