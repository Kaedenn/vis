
#ifndef VIS_ASYNC_HEADER_INCLUDED_
#define VIS_ASYNC_HEADER_INCLUDED_ 1

#include <unistd.h>

int async_setup_stdin(void);
int async_setup_stdout(void);

ssize_t async_read_stdin(char* buffer, size_t len);
ssize_t async_write_stdout(const char* buffer);

#endif

