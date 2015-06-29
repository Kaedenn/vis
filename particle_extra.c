
#include "helper.h"
#include "particle_extra.h"

pextra_t new_particle_extra(float red, float green, float blue,
                            blend_t blender) {
    pextra_t extra = DBMALLOC(sizeof(struct particle_extra));
    extra->r = red;
    extra->g = green;
    extra->b = blue;
    extra->a = 1.0f;
    extra->blender = blender;
    return extra;
}

void free_particle_extra(pextra_t extra) {
    DBFREE(extra);
}

