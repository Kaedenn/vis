
#ifndef VIS_CLARGS_HEADER_INCLUDED_
#define VIS_CLARGS_HEADER_INCLUDED_ 1

#include "defines.h"

struct clargs {
    const char* execname;
    const char* scriptfile;
    const char* scriptstring;
    const char* dumpfile;
    const char* commandfile;
    int frameskip;
    BOOL dumptrace;
    BOOL interactive;
    BOOL quiet_audio;
    BOOL stay_after_script;
    BOOL absolute_fps;

    BOOL must_exit;
    int exit_status;
};

struct clargs* argparse(int argc, char** argv);
void clargs_free(struct clargs* args);

#endif

