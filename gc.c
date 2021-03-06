
#include "helper.h"
#include "gc.h"
#include <stdlib.h>

#ifndef GC_MIN_SIZE
#define GC_MIN_SIZE 16
#endif

static void gc(void);

struct gcitem {
    gc_func func;
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

void gc_add(gc_func func, void* cls) {
    if (gcidx == ngcitems) {
        ngcitems *= 2;
        gcitems = realloc(gcitems, ngcitems);
    }
    gcitems[gcidx].func = func;
    gcitems[gcidx].cls = cls;
    ++gcidx;
}

/* free gcitems in reverse order */
void gc(void) {
    size_t i = ngcitems-1;
    do {
        if (gcitems[i].func) {
            (gcitems[i].func)(gcitems[i].cls);
        }
    } while (i--);
    DBFREE(gcitems);
}

