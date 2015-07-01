
#define _GNU_SOURCE
#include <unistd.h>

#include "clargs.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct clargs args;

const char* usage_string = "Usage: %s [-d] [-l scriptfile] [-i]\n";
const char* help_string =
"  -d <FILE> dump frames to <FILE>_000.png\n"
"  -l <FILE> run lua script <FILE>\n"
"  -i        disable interactive mode\n"
"  -h        this message\n";

void argparse(int argc, char** argv) {
    int opt;
    args.execname = argv[0];
    args.scriptfile = NULL;
    args.dumpfile = NULL;
    args.interactive = TRUE;
    if (argc > 1) {
        while ((opt = getopt(argc, argv, "d:l:ih")) != -1) {
            switch (opt) {
                case 'd':
                    args.dumpfile = optarg;
                    break;
                case 'l':
                    args.scriptfile = dupstr(optarg);
                    break;
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

#if 0
void argparse(int argc, char** argv) {
    int argi;
    args.execname = argv[0];
    args.scriptfile = NULL;
    args.debug = FALSE;
    args.interactive = TRUE;
    for (argi = 1; argi < argc; ++argi) {
        if (argv[argi][0] == '-') {
            if (!strcmp(argv[argi], "-d") ||
                !strcmp(argv[argi], "--debug")) {
                args.debug = TRUE;
            } else if (!strcmp(argv[argi], "-l") ||
                       !strcmp(argv[argi], "--load")) {
                args.scriptfile = argv[argi++];
            } else if (!strcmp(argv[argi], "-i")) {
                args.interactive = FALSE;
            } else if (!strcmp(argv[argi], "-h") ||
                       !strcmp(argv[argi], "--help")) {
                struct clopts* clopt = &clopts[0];
                printf("Usage: %s [arguments]\n", argv[0]);
                printf("Arguments:\n");
                while (clopt->cmd && clopt->help) {
                    printf("\t%s\t%s\n", clopt->cmd, clopt->help);
                    clopt++;
                }
            }
        }
        if (strcmp(argv[argi], "-d") == 0) {
            args.debug = TRUE;
        } else if (strcmp(argv[argi], "-l") == 0) {
            args.scriptfile = argv[argi + 1];
        } else if (strcmp(argv[argi], "-i") == 0) {
            args.interactive = FALSE;
        } else if (strcmp(argv[argi], "-h") == 0 ||
                   strcmp(argv[argi], "--help") == 0) {
        }
    }
    if (args.scriptfile == NULL && args.interactive == FALSE) {
        eprintf("%s error: nothing to do (no script, non-interactive)", argv[0]);
        exit(1);
    }
}
#endif

