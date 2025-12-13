
#ifndef VIS_HELPER_HEADER_INCLUDED_
#define VIS_HELPER_HEADER_INCLUDED_ 1

#include "defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* for M_PI and other stuff */
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

/* open or print the appropriate error */
FILE* try_fopen(const char* path, const char* mode);

/* malloc+assert */
void* chmalloc(size_t nbytes);
void* dbmalloc(size_t nbytes, const char* label);
void dbfree(void* ptr, const char* label);

/* helper printfs for errors or debugging */
void eprintf(const char* fmt, ...);
void dbprintf(const char* fmt, ...);
void do_assert(BOOL cond, const char* message, const char* file, int line);

#define VIS_ASSERT(cond) do_assert(((cond) ? 1 : 0), (#cond), __FILE__, __LINE__)

#if DEBUG >= DEBUG_TRACE
#define DBMALLOC(size) dbmalloc((size), (#size))
#define DBFREE(ptr) dbfree((ptr), (#ptr))
#else
#define DBMALLOC(size) chmalloc((size))
#define DBFREE(ptr) free((ptr))
#endif

#if DEBUG > DEBUG_VERBOSE
#define WHEREAMI_STDERR() fprintf(stderr, "%s:%d:%s: ", __FILE__, __LINE__, __func__)
#elif DEBUG > DEBUG_NONE
#define WHEREAMI_STDERR() fprintf(stderr, "%s:%d: ", __FILE__, __LINE__)
#else
#define WHEREAMI_STDERR()
#endif

#define EPRINTF(fmt, ...) \
    do { \
        WHEREAMI_STDERR(); \
        eprintf(fmt, __VA_ARGS__); \
    } while (0)
#define DBPRINTF(fmt, ...) \
    do { \
        WHEREAMI_STDERR(); \
        dbprintf(fmt, __VA_ARGS__); \
    } while (0)

#define ZEROINIT(p) memset(p, '\0', sizeof(*p))

/* tests if a file exists, portably */
BOOL fexists(const char* path);

/* short-circuiting strcmp */
BOOL startswith(const char* s1, const char* s2);

/* string to int */
int strtoi(const char* buff);

/* strdup isn't standard, sadly */
char* dupstr(const char* str);

/* and neither is strchrnul */
char* strnulchr(char* str, int n);

/* like strdup, but replace \n with \\n, etc */
char* escape_string(const char* str);

char* stralloc(size_t nchars);
char* allocat(char* dest, const char* source, size_t* bufsize);

BOOL parse_window_size(const char* arg, unsigned int* width, unsigned int* height);

#endif

/* vim: set ts=4 sts=4 sw=4: */
