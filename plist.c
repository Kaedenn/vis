
#include <stdlib.h>
#include <stdio.h>
#include "helper.h"
#include "plist.h"

plist_t plist_new(size_t initial_size) {
    plist_t plist = DBMALLOC(sizeof(struct plist));
    plist->particles = DBMALLOC(sizeof(particle_t) * initial_size);
    plist->size = 0;
    plist->capacity = initial_size;
    return plist;
}

void plist_free(plist_t plist) {
    size_t i;
    for (i = 0; i < plist->size; ++i) {
        particle_free((plist->particles)[i]);
    }
    DBFREE(plist->particles);
    DBFREE(plist);
}

void plist_foreach(plist_t plist, item_fn fn, void* userdefined) {
    size_t i = 0;
    while (i < plist->size) {
        plist_action_t action = (fn)(plist->particles[i], i, userdefined);
        if (action == ACTION_REMOVE) {
            /* implement swap-to-back */
            particle_t* last = &plist->particles[plist->size - 1];
            particle_free(plist->particles[i]);
            plist->particles[i] = *last;
            *last = NULL;
            plist->size -= 1;
        } else if (action == ACTION_NEXT) {
            ++i;
        }
    }
}

void plist_add(plist_t plist, particle_t p) {
    if (plist->size < plist->capacity) {
        plist->particles[plist->size++] = p;
    } else {
        /* For now, ignore */
    }
}

