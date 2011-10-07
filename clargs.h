
#ifndef VIS_CLARGS_HEADER_INCLUDED_
#define VIS_CLARGS_HEADER_INCLUDED_ 1

#include "defines.h"

extern struct clargs {
  const char* execname;
  const char* scriptfile;
  BOOL debug;
  BOOL interactive;
} args;

void argparse(int argc, char** argv);

#endif

