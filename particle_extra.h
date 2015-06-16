
#ifndef VIS_PARTICLE_EXTRA_HEADER_INCLUDED_
#define VIS_PARTICLE_EXTRA_HEADER_INCLUDED_ 1

#include "defines.h"
#include "draw.h"
#include <stdlib.h>

typedef struct particle_extra {
    float r, g, b;
    blend_t blender;
} *pextra_t;

pextra_t new_particle_extra(float red, float green, float blue,
                            blend_t blender);
void free_particle_extra(pextra_t extra);

#endif

