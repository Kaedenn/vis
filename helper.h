
#ifndef VIS_HELPER_HEADER_INCLUDED_
#define VIS_HELPER_HEADER_INCLUDED_ 1

#include "defines.h"
#include <stdio.h>

/* open or print the appropriate error */
FILE* try_fopen(const char* path, const char* mode);

/* malloc+assert */
void* chmalloc(size_t nbytes);

/* helper printfs for errors or debugging */
void eprintf(const char* fmt, ...);
void dbprintf(const char* fmt, ...);

#define DBPRINTF(fmt, ...)

#ifndef DBPRINTF
#define DBPRINTF(fmt, ...) \
  do { \
    fprintf(stderr, "%s:%d: ", __FILE__, __LINE__); \
    dbprintf(fmt, __VA_ARGS__); \
  } while (0)
#endif

#define ZEROINIT(structp) memset(structp, '\0', sizeof(*structp))

/* short-circuiting strcmp */
BOOL startswith(const char* s1, const char* s2);

/* string to int */
int strtoi(const char* buff);

/* strdup isn't standard, sadly */
char* dupstr(const char* str);

/* and neither is strchrnul */
char* strnulchr(char* str, int n);

#endif

