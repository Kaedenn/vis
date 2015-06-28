
#ifndef VIS_HELPER_HEADER_INCLUDED_
#define VIS_HELPER_HEADER_INCLUDED_ 1

#include "defines.h"
#include <stdio.h>

/* open or print the appropriate error */
FILE* try_fopen(const char* path, const char* mode);

/* malloc+assert */
void* chmalloc(size_t nbytes);
void* dbmalloc(size_t nbytes, const char* label);
void dbfree(void* ptr, const char* label);

/* helper printfs for errors or debugging */
void eprintf(const char* fmt, ...);
void dbprintf(const char* fmt, ...);

#if defined(DEBUG_MALLOC) && defined(DEBUG)
#define DBMALLOC(size) dbmalloc((size), (#size))
#define DBFREE(ptr) dbfree((ptr), (#ptr))
#else
#define DBMALLOC(size) chmalloc((size))
#define DBFREE(ptr) free((ptr))
#endif

#ifdef DEBUG
#define DBPRINTF(fmt, ...) \
    do { \
        fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); \
        dbprintf(fmt, __VA_ARGS__); \
    } while (0)
#else
#define DBPRINTF(fmt, ...) dbprintf(fmt, __VA_ARGS__)
#endif

#define ZEROINIT(p) memset(p, '\0', sizeof(*p))

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

#endif

