
#include "plist.h"
#include "helper.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

plist_t plist_new(size_t initial_size) {
    plist_t plist = DBMALLOC(sizeof(struct plist));
    plist->particles = DBMALLOC(sizeof(struct particle) * initial_size);
    plist->size = 0;
    plist->capacity = initial_size;
    return plist;
}

void plist_free(plist_t plist) {
    DZFREE(plist->particles);
    DZFREE(plist);
}

size_t plist_get_capacity(plist_t plist) {
    return plist->capacity;
}

size_t plist_get_size(plist_t plist) {
    return plist->size;
}

void plist_foreach(plist_t plist, item_fn fn, void* userdefined) {
    size_t i = 0;
    while (i < plist->size) {
        plist_action_id action = (fn)(&plist->particles[i], userdefined);
        if (action == ACTION_REMOVE) {
            if (i != plist->size - 1) {
                /* implement swap-to-back */
                plist->particles[i] = plist->particles[plist->size - 1];
            }
            plist->size -= 1;
        } else if (action == ACTION_NEXT) {
            ++i;
        }
    }
}

particle_t plist_add(plist_t plist) {
    if (plist->size < plist->capacity) {
        return &plist->particles[plist->size++];
    } else {
        EPRINTF("Attempted to add more than %lu particles to plist %p, "
                "particle dropped", (unsigned long)plist->capacity, plist);
        return NULL;
    }
}

void plist_clear(plist_t plist) {
    plist->size = 0;
}

