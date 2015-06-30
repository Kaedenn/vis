
#include "kstring.h"
#include "helper.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

kstr kstring_new(size_t capacity) {
    kstr s = DBMALLOC(sizeof(struct kstring));
    s->len = 0;
    s->capacity = capacity;
    s->content = DBMALLOC(capacity);
    return s;
}

kstr kstring_newfrom(const char* existing) {
    size_t len = strlen(existing);
    kstr s = DBMALLOC(sizeof(struct kstring));
    s->len = len;
    s->capacity = len;
    s->content = dupstr(existing);
    return s;
}

kstr kstring_newfromvf(const char* fmt, ...) {
    size_t capacity = 2;
    va_list va;
    size_t nchars;
    char* out = DBMALLOC(capacity);

    va_start(va, fmt);
    nchars = (size_t)vsnprintf(out, capacity, fmt, va);
    va_end(va);

    if (nchars > capacity) {
        capacity = nchars + 1;
        out = realloc(out, capacity);
        va_start(va, fmt);
        vsnprintf(out, capacity, fmt, va);
        va_end(va);
    }

    kstr s = kstring_newfrom(out);
    DBFREE(out);

    return s;
}

void kstring_free(kstr s) {
    DBFREE(s->content);
    DBFREE(s);
}

void kstring_append(kstr s, const char* news) {
    size_t newcap = s->capacity + strlen(news) + 1;
    size_t newlen = s->len + strlen(news) + 1;
    char* newstr = DBMALLOC(newlen);
    strcat(newstr, s->content);
    strcat(newstr, news);
    DBFREE(s->content);
    s->len = newlen;
    s->capacity = newcap;
    s->content = newstr;
}

void kstring_assimilate(kstr s1, kstr s2) {
    kstring_append(s1, kstring_content(s2));
    kstring_free(s2);
}

void kstring_realloc(kstr s, size_t newcapacity) {
    s->content = realloc(s->content, newcapacity);
    assert(s->content != NULL);
    s->capacity = newcapacity;
}

size_t kstring_length(kstr s) { return s->len; }
size_t kstring_capacity(kstr s) { return s->capacity; }
const char* kstring_content(kstr s) { return s->content; }

