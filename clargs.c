
#include "clargs.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* usage_string = "Usage: %s [-l path] [arguments...]";
const char* help_string[] = {
    " Short options:",
    "  -d <FILE> dump frames to <FILE>_000.png",
    "  -l <FILE> run lua script <FILE>",
    "  -L <LUA>  run lua string <LUA>",
    "  -f <FILE> run commands from <FILE>",
    "  -s <NUM>  skip <NUM> frames when dumping with -d",
    "  -t        output the results of tracing to stdout, implies -i",
    "  -i        disable interactive mode (commands on stdin)",
    "  -I        do not exit after a script finishes (if it calls Vis.exit)",
    "  -q        disables the playback of audio",
    "  -h        this message",
    " Long options:",
    "  --linear-fps use the old linear (self-correcting) fps limiter",
    "  --help       this message",
    "", NULL
};

static void mark_error(struct clargs* args, int error) {
    args->must_exit = TRUE;
    if (error > args->exit_status) {
        args->exit_status = error;
    }
}

struct clargs* argparse(int argc, char** argv) {
    struct clargs* args = DBMALLOC(sizeof(struct clargs));
    args->execname = argv[0];
    args->scriptfile = NULL;
    args->scriptstring = NULL;
    args->dumpfile = NULL;
    args->commandfile = NULL;
    args->frameskip = 0;
    args->dumptrace = FALSE;
    args->interactive = TRUE;
    args->absolute_fps = TRUE;
    args->quiet_audio = FALSE;
    args->stay_after_script = FALSE;
    for (int argi = 1; argi < argc && argv[argi] && argv[argi][0]; ++argi) {
        /* first non-argument argv element terminates processing */
        if (argv[argi][0] != '-' || !strcmp(argv[argi], "--")) {
            break;
        }
        switch (argv[argi][1]) {
            case 'd':
                if (argi+1 < argc) {
                    args->dumpfile = argv[++argi];
                } else {
                    EPRINTF("Argument -%s requires value", argv[argi][1]);
                    mark_error(args, 1);
                }
                break;
            case 'l':
                if (argi+1 < argc) {
                    args->scriptfile = argv[++argi];
                } else {
                    EPRINTF("Argument -%s requires value", argv[argi][1]);
                    mark_error(args, 1);
                }
                break;
            case 'L':
                if (argi+1 < argc) {
                    args->scriptstring = argv[++argi];
                } else {
                    EPRINTF("Argument -%s requires value", argv[argi][1]);
                    mark_error(args, 1);
                }
                break;
            case 'f':
                if (argi+1 < argc) {
                    args->commandfile = argv[++argi];
                } else {
                    EPRINTF("Argument -%s requires value", argv[argi][1]);
                    mark_error(args, 1);
                }
                break;
            case 's':
                if (argi+1 < argc) {
                    args->frameskip = strtoi(argv[++argi]);
                } else {
                    EPRINTF("Argument -%s requires value", argv[argi][1]);
                    mark_error(args, 1);
                }
                break;
            case 't':
                args->dumptrace = TRUE;
                break;
            case 'i':
                args->interactive = FALSE;
                break;
            case 'I':
                args->stay_after_script = TRUE;
                break;
            case 'q':
                args->quiet_audio = TRUE;
                break;
            case 'h': {
                printf(usage_string, argv[0]);
                for (size_t i = 0; help_string[i] != NULL; ++i) {
                    printf("\n%s", help_string[i]);
                }
                mark_error(args, 0);
            } break;
            case '-': /* longopt */
                if (!strcmp(argv[argi], "--linear-fps")) {
                    args->absolute_fps = FALSE;
                } else if (!strcmp(argv[argi], "--help")) {
                    printf(usage_string, argv[0]);
                    for (size_t i = 0; help_string[i] != NULL; ++i) {
                        printf("\n%s", help_string[i]);
                    }
                    mark_error(args, 0);
                } else {
                    EPRINTF("Invalid long option %s", argv[argi]);
                }
                break;
            default:
                EPRINTF("Invalid argument -%c", argv[argi][1]);
                break;
        }
    }

    if (args->frameskip < 0) {
        EPRINTF("Invalid negative frameskip %s", args->frameskip);
        mark_error(args, 1);
    }

    if (args->scriptfile && !fexists(args->scriptfile)) {
        EPRINTF("Script file %s not found or not readable", args->scriptfile);
        mark_error(args, 1);
    }

    if (args->commandfile && !fexists(args->commandfile)) {
        EPRINTF("Command file %s not found or not readable",
                args->commandfile);
        mark_error(args, 1);
    }

    return args;
}

void clargs_free(struct clargs* args) {
    DBFREE(args);
}

