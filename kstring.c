
#include "kstring.h"
#include "helper.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>

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
    assert(s);
    DBFREE(s->content);
    DBFREE(s);
}

void kstring_strip(kstr s) {
    assert(s);
    kstring_stripl(s);
    kstring_stripr(s);
}

void kstring_stripl(kstr s) {
    assert(s);
    size_t i, j;
    for (i = 0; i < s->len; ++i) {
        if (!isspace(s->content[i])) break;
    }
    if (i > 0) {
        s->len -= i;
        for (j = 0; j < s->len; ++j) {
            s->content[j] = s->content[j+i];
        }
        s->content[s->len] = '\0';
    }
}

void kstring_stripr(kstr s) {
    assert(s);
    size_t i, newlen = s->len;
    for (i = s->len-1; i > 0; --i) {
        if (isspace(s->content[i])) {
            --newlen;
        } else {
            break;
        }
    }
    s->content[newlen] = '\0';
    s->len = newlen;
}

void kstring_append(kstr s, const char* news) {
    assert(s);
    assert(news);
    size_t newcap = s->capacity + strlen(news) + 1;
    size_t newlen = s->len + strlen(news);
    char* newstr = DBMALLOC(newcap);
    strcat(newstr, s->content);
    strcat(newstr, news);
    DBFREE(s->content);
    s->len = newlen;
    s->capacity = newcap;
    s->content = newstr;
}

void kstring_assimilate(kstr s1, kstr s2) {
    assert(s1);
    assert(s2);
    kstring_append(s1, kstring_content(s2));
    kstring_free(s2);
}

void kstring_realloc(kstr s, size_t newcapacity) {
    assert(s);
    assert(newcapacity > 0);
    s->content = realloc(s->content, newcapacity);
    assert(s->content != NULL);
    s->capacity = newcapacity;
}

size_t kstring_length(kstr s) { assert(s); return s->len; }
size_t kstring_capacity(kstr s) { assert(s); return s->capacity; }
const char* kstring_content(kstr s) { assert(s); return s->content; }

