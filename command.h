
#ifndef VIS_COMMAND_HEADER_INCLUDED_
#define VIS_COMMAND_HEADER_INCLUDED_ 1

#include "drawer.h"
#include "plist.h"
#include "script.h"

typedef enum {
    CMD_ERROR_NONE,
    CMD_ERROR_RECOVERABLE,
    CMD_ERROR_FATAL
} cmd_error_id;

struct commands;

struct commands* command_setup(drawer_t drawer, plist_t plist,
                               script_t script, BOOL interactive);
void command_teardown(struct commands* commands);

/* returns an exit status if the commands module encountered an error */
BOOL command_should_exit(struct commands* commands);
cmd_error_id command_get_error(struct commands* commands);

/* handle commands via STDIN */
void command_async(struct commands* commands);

/* handle commands via input buffer */
void command_str(struct commands* commands, const char* buffer);

/* handle commands by reading a file */
void command_file(struct commands* commands, const char* file);

#endif

