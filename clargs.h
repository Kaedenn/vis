
#ifndef VIS_CLARGS_HEADER_INCLUDED_
#define VIS_CLARGS_HEADER_INCLUDED_ 1

#include "defines.h"
#include "klist.h"

typedef struct clargs {
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
} clargs;
typedef clargs* clargs_t;

clargs* argparse(int argc, char** argv);
void clargs_free(clargs* args);

#endif
