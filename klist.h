#ifndef VIS_KLIST_HEADER_INCLUDED_
#define VIS_KLIST_HEADER_INCLUDED_ 1

#include "helper.h"
#include <stdlib.h>

typedef struct klist *klist;

klist klist_new(void);
void klist_free(klist l);
void klist_realloc(klist l, size_t newcapacity);

char* klist_shift(klist l);
char* klist_pop(klist l);

void klist_unshift(klist l, const char* s);
void klist_append(klist l, const char* s);

const char* klist_getfirst(klist l);
const char* klist_getlast(klist l);
const char* klist_getn(klist l, size_t idx);

size_t klist_length(klist l);
BOOL klist_empty(klist l);

#endif
