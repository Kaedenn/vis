
#include "clargs.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct clargs args;

void argparse(int argc, char** argv) {
  int argi;
  args.execname = argv[0];
  args.scriptfile = NULL;
  args.debug = FALSE;
  args.interactive = TRUE;
  for (argi = 1; argi < argc; ++argi) {
    if (strcmp(argv[argi], "-d") == 0) {
      args.debug = TRUE;
    } else if (strcmp(argv[argi], "-l") == 0) {
      args.scriptfile = argv[argi + 1];
    } else if (strcmp(argv[argi], "-i") == 0) {
      args.interactive = FALSE;
    }
  }
  if (args.scriptfile == NULL && args.interactive == FALSE) {
    eprintf("%s error: nothing to do (no script, non-interactive)", argv[0]);
    exit(1);
  }
}

