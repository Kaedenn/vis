
#include "klist.h"
#include "helper.h"

struct klist_node {
    struct klist_node* prev;
    struct klist_node* next;
    char* str;
};

struct klist {
    size_t length;
    struct klist_node* head;
    struct klist_node* tail;
};

static struct klist_node* klist_node_new(const char* str) {
    struct klist_node* newnode = DBMALLOC(sizeof(struct klist_node));
    newnode->str = dupstr(str);
    return newnode;
}

static void klist_node_free(struct klist_node* node) {
    if (node && node->str) {
        DBFREE(node->str);
    }
    DBFREE(node);
}

klist klist_new(void) {
    return DBMALLOC(sizeof(struct klist));
}

void klist_free(klist l) {
    while (!klist_empty(l)) {
        DBFREE(klist_shift(l));
    }
    DBFREE(l);
}

char* klist_shift(klist l) {
    struct klist_node* head = l->head;
    char* content = head ? dupstr(head->str) : NULL;
    if (head == NULL) {
        VIS_ASSERT(l->length == 0);
        VIS_ASSERT(l->tail == NULL);
    } else if (l->tail == head) {
        VIS_ASSERT(l->length == 1);
        l->head = l->tail = NULL;
        l->length -= 1;
    } else {
        VIS_ASSERT(l->length > 1);
        l->head = head->next;
        l->head->prev = NULL;
        l->length -= 1;
    }
    klist_node_free(head);
    return content;
}

char* klist_pop(klist l) {
    struct klist_node* tail = l->tail;
    char* content = tail ? dupstr(tail->str) : NULL;
    if (tail == NULL) {
        VIS_ASSERT(l->length == 0);
        VIS_ASSERT(l->head == NULL);
    } else if (l->head == tail) {
        VIS_ASSERT(l->length == 1);
        l->head = l->tail = NULL;
        l->length -= 1;
    } else {
        VIS_ASSERT(l->length > 1);
        l->tail = l->tail->prev;
        l->tail->next = NULL;
        l->length -= 1;
    }
    klist_node_free(tail);
    return content;
}

void klist_unshift(klist l, const char* s) {
    struct klist_node* newnode = klist_node_new(s);
    if (l->head == NULL) {
        VIS_ASSERT(klist_empty(l));
        l->head = l->tail = newnode;
    } else {
        VIS_ASSERT(!klist_empty(l));
        newnode->next = l->head;
        l->head->prev = newnode;
        l->head = newnode;
    }
    l->length += 1;
}

void klist_append(klist l, const char* s) {
    struct klist_node* newnode = klist_node_new(s);
    if (l->tail == NULL) {
        VIS_ASSERT(klist_empty(l));
        l->head = l->tail = newnode;
    } else {
        VIS_ASSERT(!klist_empty(l));
        newnode->prev = l->tail;
        l->tail->next = newnode;
        l->tail = newnode;
    }
    l->length += 1;
}

const char* klist_getfirst(klist l) {
    return !klist_empty(l) ? l->head->str : NULL;
}

const char* klist_getlast(klist l) {
    return !klist_empty(l) ? l->tail->str : NULL;
}

const char* klist_getn(klist l, size_t idx) {
    if (idx >= l->length) {
        EPRINTF("Attempt to get pass-the-end idx %d on klist %p", idx, l);
        return NULL;
    }
    struct klist_node* curr = l->head;
    for (size_t i = 0; i < idx; ++i) {
        VIS_ASSERT(curr && curr->next);
        curr = curr->next;
    }
    return curr->str;
}

size_t klist_length(klist l) {
    return l->length;
}

BOOL klist_empty(klist l) {
    /* both true or both false */
    if (l->length == 0) {
        VIS_ASSERT(l->head == l->tail && l->head == NULL);
    } else if (l->length == 1) {
        VIS_ASSERT(l->head == l->tail && l->head != NULL);
    } else {
        VIS_ASSERT(l->head != l->tail && l->head != NULL && l->tail != NULL);
    }
    return l->length == 0;
}
