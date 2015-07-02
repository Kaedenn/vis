
#include "helper.h"
#include "gc.h"
#include <stdlib.h>

#ifndef GC_MIN_SIZE
#define GC_MIN_SIZE 16
#endif

static void gc(void);

struct gcitem {
    gc_func_t func;
    void* cls;
};

static struct gcitem* gcitems = NULL;
static size_t gcidx = 0;
static size_t ngcitems = 0;

void gc_init(void) {
    ngcitems = GC_MIN_SIZE;
    gcitems = DBMALLOC(ngcitems * sizeof(struct gcitem));
    atexit(gc);
}

void gc_add(gc_func_t func, void* cls) {
    if (gcidx == ngcitems) {
        ngcitems *= 2;
        gcitems = realloc(gcitems, ngcitems);
    }
    gcitems[gcidx].func = func;
    gcitems[gcidx].cls = cls;
    ++gcidx;
}

void gc(void) {
    for (size_t i = 0; i < ngcitems; ++i) {
        if (gcitems[i].func) {
            (gcitems[i].func)(gcitems[i].cls);
        }
    }
    DBFREE(gcitems);
}

