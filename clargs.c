
#include "clargs.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* usage_string = "Usage: %s [-d path] [-l path] [-t] [-i] [-q] [-h]\n";
const char* help_string =
"  -d <FILE> dump frames to <FILE>_000.png\n"
"  -l <FILE> run lua script <FILE>\n"
"  -t        output the results of tracing to stdout, implies -i\n"
"  -i        disable interactive mode (commands on stdin)\n"
"  -q        disables the playback of audio\n"
"  -h        this message\n";

struct clargs* argparse(int argc, char** argv) {
    struct clargs* clargs = DBMALLOC(sizeof(struct clargs));
    clargs->execname = argv[0];
    clargs->scriptfile = NULL;
    clargs->dumpfile = NULL;
    clargs->frameskip = 0;
    clargs->dumptrace = FALSE;
    clargs->interactive = TRUE;
    clargs->enlarge_particles = FALSE;
    clargs->quiet_audio = FALSE;
    for (int argi = 1; argi < argc && argv[argi] && argv[argi][0]; ++argi) {
        if (argv[argi][0] == '-') {
            switch (argv[argi][1]) {
                case 'd':
                    if (argi+1 < argc) {
                        clargs->dumpfile = argv[++argi];
                    } else {
                        eprintf("Argument -d requires value");
                        clargs->must_exit = TRUE;
                        clargs->exit_status = 1;
                    }
                    break;
                case 'l':
                    if (argi+1 < argc) {
                        clargs->scriptfile = argv[++argi];
                    } else {
                        eprintf("Argument -l requires value");
                        clargs->must_exit = TRUE;
                        clargs->exit_status = 1;
                    }
                    break;
                case 's':
                    if (argi+1 < argc) {
                        clargs->frameskip = strtoi(argv[++argi]);
                    } else {
                        eprintf("Argument -s requires value");
                        clargs->must_exit = TRUE;
                        clargs->exit_status = 1;
                    }
                    break;
                case 't':
                    clargs->dumptrace = TRUE;
                    break;
                case 'i':
                    clargs->interactive = FALSE;
                    break;
                case 'e':
                    clargs->enlarge_particles = TRUE;
                    break;
                case 'q':
                    clargs->quiet_audio = TRUE;
                    break;
                case 'h':
                    printf(usage_string, argv[0]);
                    printf("%s", help_string);
                    clargs->must_exit = TRUE;
                    break;
                default:
                    eprintf("Invalid argument -%c", argv[argi][1]);
                    break;
            }
        }
    }
    return clargs;
}

