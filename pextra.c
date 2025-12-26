
#include "helper.h"
#include "pextra.h"
#include <stdlib.h>

pextra* new_pextra(float red, float green, float blue, blend_id blender) {
    pextra* extra = DBMALLOC(sizeof(struct pextra));
    extra->r = red;
    extra->g = green;
    extra->b = blue;
    extra->a = 1.0f;
    extra->blender = blender;
    return extra;
}

void free_pextra(pextra* extra) {
    DBFREE(extra);
}

uint64_t pextra_hash_string(const char* string) {
    uint64_t result = 0;
    for (unsigned i = 0; string[i] != '\0'; ++i) {
        result = (result << 8) | (unsigned char)string[i];
    }
    return result;
}

