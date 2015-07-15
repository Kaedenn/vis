
#ifndef VIS_PARTICLE_LIST_HEADER_INCLUDED_
#define VIS_PARTICLE_LIST_HEADER_INCLUDED_

#include "particle.h"

typedef struct plist {
    struct particle** particles;
    size_t size;
    size_t capacity;
} *plist_t;

typedef enum plist_action {
    ACTION_NEXT,
    ACTION_REMOVE
} plist_action_id;

typedef plist_action_id (*item_fn)(struct particle* p, size_t idx, void* userdefined);

plist_t plist_new(size_t initial_size);
void plist_free(plist_t plist);

size_t plist_get_capacity(plist_t plist);
size_t plist_get_size(plist_t plist);

void plist_foreach(plist_t plist, item_fn fn, void* userdefined);

void plist_add(plist_t plist, struct particle* p);
void plist_clear(plist_t plist);

#endif

