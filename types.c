
#include "types.h"
#include "helper.h"

void dbprintf_emit_t(emit_t e) {
    DBPRINTF("struct *emit_t { n=%d, x=%g, y=%g, ux=%g, uy=%g, "
             "rad=%g, urad=%g, life=%d, ulife=%d, ds=%g, uds=%g, "
             "theta=%g, utheta=%g, r=%g, g=%g, b=%g, "
             "ur=%g, ug=%g, ub=%g, force=%d, limit=%d, blender=%d };",
             e->n, e->x, e->y, e->ux, e->uy, e->rad, e->urad,
             e->life, e->ulife, e->ds, e->uds, e->theta, e->utheta,
             (double)e->r, (double)e->g, (double)e->b,
             (double)e->ur, (double)e->ug, (double)e->ub,
             e->force, e->limit, e->blender);
}
