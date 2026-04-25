#ifndef VIS_KSTRING_HEADER_INCLUDED_
#define VIS_KSTRING_HEADER_INCLUDED_ 1

#include <stdlib.h>

typedef struct kstring {
    size_t len;
    size_t capacity;
    char* content;
} *kstr;

kstr kstring_new(size_t capacity);
kstr kstring_newfrom(const char* existing);
kstr kstring_newfromvf(const char* fmt, ...);

void kstring_free(kstr s);

void kstring_strip(kstr s);
void kstring_stripl(kstr s);
void kstring_stripr(kstr s);

void kstring_append(kstr s, const char* news);
void kstring_assimilate(kstr s1, kstr s2); /* frees s2 */

#define kstring_appendvf(s, f, ...) \
    (kstring_assimilate(s, (kstring_newfromvf(f, __VA_ARGS__))))

void kstring_realloc(kstr s, size_t newcapacity);

BOOL kstring_empty(const kstr s);
size_t kstring_length(const kstr s);
size_t kstring_capacity(const kstr s);
const char* kstring_content(const kstr s);
/* use dupstr(kstring_content(s)) to get a copy */

#endif
