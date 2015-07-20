
#ifndef VIS_PARTICLE_EXTRA_HEADER_INCLUDED_
#define VIS_PARTICLE_EXTRA_HEADER_INCLUDED_ 1

#include "defines.h"
#include "drawer.h"
#include <stdint.h>

typedef struct pextra {
    float r, g, b, a;
    blend_id blender;
    union particle_tag tag;
} pextra;

pextra* new_pextra(float red, float green, float blue,
                            blend_id blender);
void free_pextra(pextra* extra);

#endif

