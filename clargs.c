
#define _GNU_SOURCE
#include <unistd.h>

#include "clargs.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct clargs args;

const char* usage_string = "Usage: %s [-d path] [-l path] [-t] [-i] [-h]\n";
const char* help_string =
"  -d <FILE> dump frames to <FILE>_000.png\n"
"  -l <FILE> run lua script <FILE>\n"
"  -t        output the results of tracing to stdout, implies -i\n"
"  -i        disable interactive mode\n"
"  -h        this message\n";

void argparse(int argc, char** argv) {
    int opt;
    args.execname = argv[0];
    args.scriptfile = NULL;
    args.dumpfile = NULL;
    args.dumptrace = FALSE;
    args.interactive = TRUE;
    if (argc > 1) {
        while ((opt = getopt(argc, argv, "d:l:tih")) != -1) {
            switch (opt) {
                case 'd':
                    args.dumpfile = optarg;
                    break;
                case 'l':
                    args.scriptfile = dupstr(optarg);
                    break;
                case 't':
                    args.dumptrace = TRUE;
                    /* fall through */
                case 'i':
                    args.interactive = FALSE;
                    break;
                case 'h':
                    printf(usage_string, argv[0]);
                    printf("%s", help_string);
                    exit(0);
                case '?':
                default:
                    eprintf("Invalid argument %s", argv[optind-1]);
                    eprintf("Usage: %s [-d] [-l scriptfile] [-i]", argv[0]);
                    exit(1);
            }
        }
    }
}

