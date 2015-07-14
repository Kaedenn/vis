
#ifndef VIS_PARTICLE_EXTRA_HEADER_INCLUDED_
#define VIS_PARTICLE_EXTRA_HEADER_INCLUDED_ 1

#include "defines.h"
#include "drawer.h"
#include <stdint.h>

typedef struct particle_extra {
    float r, g, b, a;
    blend_id blender;
    union {
        char c[8];
        int64_t l;
    } tag;
} *pextra_t;

pextra_t new_particle_extra(float red, float green, float blue,
                            blend_id blender);
void free_particle_extra(pextra_t extra);

#endif

