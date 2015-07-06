
#ifndef VIS_COMMAND_HEADER_INCLUDED_
#define VIS_COMMAND_HEADER_INCLUDED_ 1

#include "drawer.h"
#include "plist.h"
#include "script.h"

struct commands;

struct commands* command_setup(drawer_t drawer, plist_t plist, script_t script);
void command_teardown(struct commands* commands);

/* handle commands via STDIN */
void command(struct commands* commands);

/* handle commands via input buffer */
void docommand(struct commands* commands, const char* buffer);

#endif

