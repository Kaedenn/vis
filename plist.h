
#ifndef VIS_PARTICLE_LIST_HEADER_INCLUDED_
#define VIS_PARTICLE_LIST_HEADER_INCLUDED_

#include "particle.h"

typedef struct plist_node {
  struct plist_node* prev;
  struct plist_node* next;
  particle_t particle;
} *plist_node_t;

typedef struct plist {
  plist_node_t head;
  plist_node_t tail;
  size_t size;
} *plist_t;

typedef void (*item_fn)(plist_node_t node);

plist_t plist_new(void);
void plist_free(plist_t plist);

void plist_safe_foreach(plist_t plist, item_fn fn);

void plist_add(plist_t plist, particle_t p);
void plist_remove(plist_t plist, plist_node_t node);
void plist_clear(plist_t plist);

#endif

