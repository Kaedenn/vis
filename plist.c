
#include <stdlib.h>
#include <stdio.h>
#include "helper.h"
#include "plist.h"

static plist_node_t plist_node_new(particle_t p) {
  plist_node_t node = chmalloc(sizeof(struct plist_node));
  node->particle = p;
  node->prev = NULL;
  node->next = NULL;
  return node;
}

plist_t plist_new(void) {
  plist_t plist = chmalloc(sizeof(struct plist));
  plist->head = plist->tail = NULL;
  plist->size = 0;
  return plist;
}

void plist_free(plist_t plist) {
  plist_clear(plist);
  free(plist);
}

void plist_safe_foreach(plist_t plist, item_fn fn) {
  plist_node_t curr;
  plist_node_t next;
  if (plist->head != NULL) {
    if (plist->head != plist->tail) {
      curr = plist->head;
      while (curr != NULL) {
        next = curr->next;
        fn(curr);
        curr = next;
      }
    } else {
      fn(plist->head);
    }
  }
}

void plist_add(plist_t plist, particle_t p) {
  plist_node_t node = plist_node_new(p);
  if (plist->head == NULL) {
    plist->head = plist->tail = node;
  } else {
    plist->tail->next = node;
    node->prev = plist->tail;
    plist->tail = node;
  }
  plist->size += 1;
}

void plist_remove(plist_t plist, plist_node_t node) {
  if (node == plist->head && node == plist->tail) {
    particle_free(node->particle);
    free(node);
    plist->head = plist->tail = NULL;
  } else if (node == plist->head) {
    plist->head = plist->head->next;
    plist->head->prev = NULL;
    particle_free(node->particle);
    free(node);
  } else if (node == plist->tail) {
    plist->tail = plist->tail->prev;
    plist->tail->next = NULL;
    particle_free(node->particle);
    free(node);
  } else {
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    particle_free(node->particle);
    free(node);
  }
  plist->size -= 1;
}

void plist_clear(plist_t plist) {
  while (plist->head != NULL) {
    plist_remove(plist, plist->head);
  }
}

