
#include "clargs.h"
#include "helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* usage_string = "Usage: %s [-l path] [arguments...]";
const char* help_string[] = {
    /* clang-format off */
    " Short options:",
    "  -d <FILE>    dump frames to <FILE>_000.png",
    "  -l <FILE>    run lua script <FILE>",
    "  -L <LUA>     run lua string <LUA>",
    "  -A <ARG>     pass <ARG> to Lua; can be specified more than once",
    "  -f <FILE>    run commands from <FILE>",
    "  -s <NUM>     skip <NUM> frames when dumping with -d",
    "  -F <NUM>     change the framerate to <NUM>",
    "  -w <W>x<H>   set window size to <W>x<H>",
    "  -t           output the results of tracing to stdout, implies -i",
    "  -i           disable interactive mode (commands on stdin)",
    "  -I           do not exit after a script finishes (if it calls Vis.exit)",
    "  -q           disables the playback of audio",
    "  -v           enable verbose output",
    "  -V           enable very verbose output",
    "  -h           print this message and exit",
    " Long options:",
    "  --linear-fps use the old linear (self-correcting) fps limiter",
    "  --help       print this message and exit",
    " Other stuff:",
    "  --           everything following is sent to Lua (like -A)",
    "",
    NULL
    /* clang-format on */
};

static void mark_error(clargs* args, int error) {
    args->must_exit = TRUE;
    if (error > args->exit_status) {
        args->exit_status = error;
    }
}

clargs* argparse(int argc, char** argv) {
    clargs* args = DBMALLOC(sizeof(struct clargs));
    args->execname = argv[0];
    args->scriptfile = NULL;
    args->scriptstring = NULL;
    args->dumpfile = NULL;
    args->commandfile = NULL;
    args->scriptargs = klist_new();
    args->frameskip = 0;
    args->frames_per_second = VIS_FPS_LIMIT;
    args->dumptrace = FALSE;
    args->interactive = TRUE;
    args->absolute_fps = TRUE;
    args->wsize[0] = VIS_WIDTH;
    args->wsize[1] = VIS_HEIGHT;
    args->quiet_audio = FALSE;
    args->stay_after_script = FALSE;
    args->debug_level = 0;
    klist unrecognized = klist_new();
    int argi;
    for (argi = 1; argi < argc && argv[argi] && argv[argi][0]; ++argi) {
        if (argv[argi][0] != '-') {
            klist_append(unrecognized, argv[argi]);
            continue;
        } else if (!strcmp(argv[argi], "--")) {
            for (++argi; argi < argc; ++argi) {
                klist_append(args->scriptargs, argv[argi]);
            }
            break;
        }
        switch (argv[argi][1]) {
        case 'd':
            if (argi + 1 < argc) {
                args->dumpfile = argv[++argi];
            } else {
                EPRINTF("Argument -%c requires value", argv[argi][1]);
                mark_error(args, 1);
            }
            break;
        case 'F':
            if (argi + 1 < argc) {
                args->frames_per_second = atoi(argv[++argi]);
                if (args->frames_per_second <= 0) {
                    EPRINTF("Value for -F must be positive; got %s", argv[argi]);
                    mark_error(args, 1);
                }
            } else {
                EPRINTF("Argument -%c requires value", argv[argi][1]);
                mark_error(args, 1);
            }
            break;
        case 'w':
            if (argi + 1 < argc) {
                parse_wsize(
                    argv[++argi], &args->wsize[0], &args->wsize[1]);
            } else {
                EPRINTF("Argument -%c requires value", argv[argi][1]);
                mark_error(args, 1);
            }
            break;
        case 'l':
            if (argi + 1 < argc) {
                args->scriptfile = argv[++argi];
            } else {
                EPRINTF("Argument -%c requires value", argv[argi][1]);
                mark_error(args, 1);
            }
            break;
        case 'L':
            if (argi + 1 < argc) {
                args->scriptstring = argv[++argi];
            } else {
                EPRINTF("Argument -%c requires value", argv[argi][1]);
                mark_error(args, 1);
            }
            break;
        case 'A':
            if (argi + 1 < argc) {
                klist_append(args->scriptargs, argv[++argi]);
            } else {
                EPRINTF("Argument -%c requires value", argv[argi][1]);
                mark_error(args, 1);
            }
            break;
        case 'f':
            if (argi + 1 < argc) {
                args->commandfile = argv[++argi];
            } else {
                EPRINTF("Argument -%c requires value", argv[argi][1]);
                mark_error(args, 1);
            }
            break;
        case 's':
            if (argi + 1 < argc) {
                args->frameskip = strtoi(argv[++argi]);
            } else {
                EPRINTF("Argument -%c requires value", argv[argi][1]);
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
        case 'v':
            args->debug_level = 1;
            break;
        case 'V':
            args->debug_level = 2;
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

    for (size_t i = 0; i < klist_length(unrecognized); ++i) {
        EPRINTF("Unrecognized argument %s", klist_getn(unrecognized, i));
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
        EPRINTF("Command file %s not found or not readable", args->commandfile);
        mark_error(args, 1);
    }

    klist_free(unrecognized);

    return args;
}

void clargs_free(clargs* args) {
    DZFREE(args);
}
