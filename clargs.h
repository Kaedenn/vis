
#ifndef VIS_CLARGS_HEADER_INCLUDED_
#define VIS_CLARGS_HEADER_INCLUDED_ 1

#include "defines.h"

struct clargs {
    const char* execname;
    const char* scriptfile;
    const char* dumpfile;
    BOOL dumptrace;
    BOOL interactive;
    BOOL enlarge_particles;
};

struct clargs* argparse(int argc, char** argv);

#endif

