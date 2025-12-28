
#include "helper.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* try_fopen(const char* path, const char* mode) {
    FILE* fp = fopen(path, mode);
    if (fp == NULL) {
        fprintf(stderr, "unable to open `%s`: %s\n", path, strerror(errno));
    }
    return fp;
}

char* stralloc(size_t nchars) {
    char* buffer = calloc(nchars, 1);
    if (!buffer) {
        fprintf(stderr, "alloc %ld failed: %d %s\n", (long)nchars, errno,
                strerror(errno));
        do_assert(buffer != NULL, "Allocation failed", __FILE__, __LINE__);
    }
    return buffer;
}

char* strrealloc(char* buffer, size_t newsize) {
    char* newbuffer = realloc((void*)buffer, newsize);
    if (!newbuffer) {
        fprintf(
            stderr, "realloc %ld failed: %d %s\n", (long)newsize, errno,
            strerror(errno));
        do_assert(newbuffer != NULL, "Reallocation failed", __FILE__, __LINE__);
    }
    return newbuffer;
}

char* allocat(char* dest, const char* source, size_t* bufsize) {
    do_assert(dest != NULL, "dest is NULL", __FILE__, __LINE__);
    do_assert(source != NULL, "source is NULL", __FILE__, __LINE__);
    do_assert(bufsize != NULL, "bufsize is NULL", __FILE__, __LINE__);
    const size_t dsize = strlen(dest);
    const size_t ssize = strlen(source);
    if (dsize + ssize >= *bufsize) {
        *bufsize = *bufsize * 2;
        dest = strrealloc(dest, *bufsize);
    }
    strncat(dest, source, *bufsize - dsize);
    return dest;
}

void* chmalloc(size_t nbytes) {
    void* p = malloc(nbytes);
    VIS_ASSERT(p != NULL);
    memset(p, 0, nbytes);
    return p;
}

void* dbmalloc(size_t nbytes, const char* label) {
    void* ptr = chmalloc(nbytes);
    DBPRINTF("Allocated %d bytes for %s: %p", nbytes, label, ptr);
    return ptr;
}

void dbfree(void* ptr, const char* label) {
    DBPRINTF("Freeing %p for %s", ptr, label);
    free(ptr);
}

void eprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

static void do_assert_fail(const char* message, const char* file, int line)
    NORETURN;

static void do_assert_fail(const char* message, const char* file, int line) {
    eprintf("Assertion failure: %s:%d: %s", file, line, message);
    abort();
}

void do_assert(BOOL cond, const char* message, const char* file, int line) {
    if (!cond) {
        do_assert_fail(message, file, line);
    }
#if DEBUG > DEBUG_DEBUG
    else {
        DBPRINTF("Assert passes: %s:%d: %s", file, line, message);
    }
#endif
}

void dbprintf(const char* fmt, ...) {
#if DEBUG > DEBUG_NONE
    va_list args;
    fprintf(stderr, "DEBUG: ");
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
#else
    (void)fmt;
#endif
}

BOOL fexists(const char* path) {
    int olderrno = errno;
    BOOL exists = FALSE;
    errno = 0;
    FILE* fp = fopen(path, "r+");
    if (fp) {
        exists = TRUE;
        fclose(fp);
    } else if (errno != ENOENT) {
        exists = TRUE;
    }
    errno = olderrno;
    return exists;
}

BOOL startswith(const char* s1, const char* s2) {
    return s1 && s2 && strncmp(s1, s2, strlen(s2)) == 0;
}

int strtoi(const char* buff) {
    return (int)strtol(buff, NULL, 10);
}

char* dupstr(const char* str) {
    char* newstr = DBMALLOC(strlen(str) + 1);
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
            if (dest)
                dest[j++] = src[i];
            ++len;
        } else {
            if (dest)
                dest[j++] = '\\';
            switch (src[i]) {
            case '\r':
                if (dest)
                    dest[j++] = 'r';
                len += 2;
                break;
            case '\n':
                if (dest)
                    dest[j++] = 'n';
                len += 2;
                break;
            case '\v':
                if (dest)
                    dest[j++] = 'v';
                len += 2;
                break;
            case '\f':
                if (dest)
                    dest[j++] = 'f';
                len += 2;
                break;
            case '\t':
                if (dest)
                    dest[j++] = 't';
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
        dest[len - 1] = '\0';
    }
    return len;
}

char* escape_string(const char* str) {
    size_t len;
    char* result = NULL;
    len = escape_count_copy(NULL, str);
    result = DBMALLOC(len);
    VIS_ASSERT(len == escape_count_copy(result, str));
    return result;
}

BOOL parse_wsize(const char* arg, unsigned* width, unsigned* height) {
    VIS_ASSERT(arg != NULL);
    VIS_ASSERT(width != NULL);
    VIS_ASSERT(height != NULL);
    int result = sscanf(arg, "%ux%u", width, height);
    return result == 2;
}

int clampi(int value, int low, int high) {
    return value < low ? low : (value > high ? high : value);
}

float clampf(float value, float low, float high) {
    return value < low ? low : (value > high ? high : value);
}

double clampd(double value, double low, double high) {
    return value < low ? low : (value > high ? high : value);
}

float lerpf(float v0, float v1, float t) {
    return v0 + t * (v1 - v0);
}

double lerpd(double v0, double v1, double t) {
    return v0 + t * (v1 - v0);
}


