
#ifndef VIS_COMMAND_HEADER_INCLUDED_
#define VIS_COMMAND_HEADER_INCLUDED_ 1

#include "plist.h"

void command_setup(plist_t plist);
void command_teardown(void);

/* handle commands via STDIN */
void command(void);

/* handle commands via input buffer */
void docommand(const char* buffer);

#endif

