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

void kstring_append(kstr s, const char* news);
void kstring_assimilate(kstr s1, kstr s2); /* frees s2 */

#define kstring_appendvf(s, f, ...) (kstring_assimilate(s, (kstring_newfromvf(f, __VA_ARGS__))))

void kstring_realloc(kstr s, size_t newcapacity);

size_t kstring_length(kstr s);
size_t kstring_capacity(kstr s);
const char* kstring_content(kstr s);
/* use dupstr(kstring_content(s)) to get a copy */

#endif
