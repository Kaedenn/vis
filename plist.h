
#ifndef VIS_PARTICLE_LIST_HEADER_INCLUDED_
#define VIS_PARTICLE_LIST_HEADER_INCLUDED_

#include "particle.h"

typedef struct plist {
    particle_t* particles;
    size_t size;
    size_t capacity;
} *plist_t;

typedef enum plist_action {
    ACTION_NEXT,
    ACTION_REMOVE
} plist_action_t;

typedef plist_action_t (*item_fn)(particle_t p, size_t idx, void* userdefined);

plist_t plist_new(size_t initial_size);
void plist_free(plist_t plist);

void plist_foreach(plist_t plist, item_fn fn, void* userdefined);

void plist_add(plist_t plist, particle_t p);
void plist_clear(plist_t plist);

#endif

