
#include <fcntl.h>
#include <string.h>

#include "async.h"

int async_setup_stdin(void) {
    return fcntl(STDIN_FILENO, F_SETFL,
                 fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);
}

int async_setup_stdout(void) {
    return fcntl(STDOUT_FILENO, F_SETFL,
                 fcntl(STDOUT_FILENO, F_GETFL) | O_NONBLOCK);
}

ssize_t async_read_stdin(char* buffer, size_t len) {
    return read(STDIN_FILENO, buffer, len);
}

ssize_t async_write_stdout(const char* buffer) {
    return write(STDOUT_FILENO, buffer, strlen(buffer));
}

