
#include "helper.h"
#include "pextra.h"

pextra* new_pextra(float red, float green, float blue,
                            blend_id blender) {
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

