#ifndef VIS_KSTRING_HEADER_INCLUDED_
#define VIS_KSTRING_HEADER_INCLUDED_ 1

/** KString library for simple managed string operations.
 *
 * This module aims to provide memory-safe buffer operations on
 * character arrays. 
 */

#include "defines.h"
#include <stdlib.h>

typedef struct kstring {
    size_t len;
    size_t capacity;
    char* content;
} *kstr;

/* Construct a kstring with a given initial capacity */
kstr kstring_new(size_t capacity);

/* Construct a kstring from a static string */
kstr kstring_newfrom(const char* existing);

/* Construct a kstring from a printf()-style format sequence */
kstr kstring_newfromvf(const char* fmt, ...);

/* Obligatory destructor */
void kstring_free(kstr s);

/* Remove whitespace at both beginning and end */
void kstring_strip(kstr s);
/* Remove whitespace at the beginning */
void kstring_stripl(kstr s);
/* Remove whitespace at the end */
void kstring_stripr(kstr s);
/* Trim offset characters off the beginning of s */
void kstring_substr_l(kstr s, size_t offset);
/* Trim offset characters off the end of s */
void kstring_substr_r(kstr s, size_t offset);

/* Append a static string */
void kstring_append(kstr s, const char* news);

/* Append s2 to s1, freeing s2 */
void kstring_assimilate(kstr s1, kstr s2);

/* If s starts with text, remove it and return TRUE. Return FALSE otherwise */
BOOL kstring_ltrim(kstr s, const char* text);

/* If s ends with text, remove it and return TRUE. Return FALSE otherwise */
BOOL kstring_rtrim(kstr s, const char* text);

/* Convenience macro to append a printf()-style format sequence to s */
#define kstring_appendvf(s, f, ...) \
    (kstring_assimilate(s, (kstring_newfromvf(f, __VA_ARGS__))))

/* Change how large the underlying buffer is */
void kstring_realloc(kstr s, size_t newcapacity);

/* TRUE if s is empty, FALSE otherwise */
BOOL kstring_empty(const kstr s);

/* Obtain the current length of s */
size_t kstring_length(const kstr s);

/* Obtain the current buffer size of s */
size_t kstring_capacity(const kstr s);

/* Obtain the content of s. Do not free() the returned pointer.
 * Use dupstr(kstring_content(s)) if you need a copy */
const char* kstring_content(const kstr s);

#endif
