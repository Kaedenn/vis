
#include "helper.h"

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

FILE* try_fopen(const char* path, const char* mode) {
    FILE* fp = fopen(path, mode);
    if (fp == NULL) {
        fprintf(stderr, "unable to open `%s`: %s\n", path, strerror(errno));
    }
    return fp;
}

void* chmalloc(size_t nbytes) {
    void* p = malloc(nbytes);
    assert(p != NULL);
    memset(p, 0, nbytes);
    return p;
}

void eprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void dbprintf(const char* fmt, ...) {
    va_list args;
    fprintf(stdout, "debug: ");
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}

BOOL startswith(const char* s1, const char* s2) {
    return s1 && s2 && strncmp(s1, s2, strlen(s2)) == 0;
}

int strtoi(const char* buff) {
    return (int)strtol(buff, NULL, 10);
}

char* dupstr(const char* str) {
    char* newstr = chmalloc(strlen(str) + 1);
    strcpy(newstr, str);
    return newstr;
}

char* strnulchr(char* str, int n) {
    char* result = strchr(str, n);
    if (result == NULL) {
        return str + strlen(str);
    } else {
        return result;
    }
}

static size_t escape_count_copy(char* dest, const char* src) {
    size_t len = 1; /* for trailing \0 */
    size_t i, j;
    for (i = 0, j = 0; src[i] != '\0'; ++i) {
        if (src[i] >= ' ' && src[i] <= '~') {
            if (dest) dest[j++] = src[i];
            ++len;
        } else {
            if (dest) dest[j++] = '\\';
            switch (src[i]) {
                case '\r':
                    if (dest) dest[j++] = 'r';
                    len += 2;
                    break;
                case '\n':
                    if (dest) dest[j++] = 'n';
                    len += 2;
                    break;
                case '\v':
                    if (dest) dest[j++] = 'v';
                    len += 2;
                    break;
                case '\f':
                    if (dest) dest[j++] = 'f';
                    len += 2;
                    break;
                case '\t':
                    if (dest) dest[j++] = 't';
                    len += 2;
                    break;
            default:
                    dest[j++] = 'x';
                    dest[j++] = (char)((src[i] / 10) % 10 + '0');
                    dest[j++] = (char)(src[i] % 10 + '0');
                    len += 4; /* \xNN */
                    break;
            }
        }
    }
    if (dest) {
        dest[len-1] = '\0';
    }
    return len;
}

char* escape_string(const char* str) {
    size_t len = 1; /* for trailing \0 */
    char* result = NULL;
    size_t i, j;
    char c;
    len = escape_count_copy(NULL, str);
    result = chmalloc(len);
    assert(len == escape_count_copy(result, str));
    return result;
}

