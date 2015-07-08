
#include "genlua.h"
#include "kstring.h"
#include "types.h"
#include "emit.h"
#include "defines.h"
#include "drawer.h"

char* genlua_emit(emit_t emit, fnum_t when) {
    char* result;
    kstr s = kstring_newfrom("Vis.emit(");
    kstring_append(s, "Vis.flist, ");
    kstring_appendvf(s, "%d, Vis.frames2msec(%d), ", emit->n, when);
    kstring_appendvf(s, "%g, %g, %g, %g, ", emit->x, emit->y, emit->ux,
                     emit->uy);
    kstring_appendvf(s, "%g, %g, ", emit->rad, emit->urad);
    kstring_appendvf(s, "%g, %g, ", emit->ds, emit->uds);
    kstring_appendvf(s, "%g, %g, ", emit->theta, emit->utheta);
    kstring_appendvf(s, "Vis.frames2msec(%d), Vis.frames2msec(%d), ",
                     emit->life, emit->ulife);
    kstring_appendvf(s, "%g, %g, %g, ", (double)emit->r, (double)emit->g,
                     (double)emit->b);
    kstring_appendvf(s, "%g, %g, %g, ", (double)emit->ur, (double)emit->ug,
                     (double)emit->ub);
    kstring_appendvf(s, "%s, ", genlua_force(emit->force));
    kstring_appendvf(s, "%s, ", genlua_limit(emit->limit));
    kstring_appendvf(s, "%s)", genlua_blender(emit->blender));

    result = dupstr(kstring_content(s));
    kstring_free(s);
    return result;
}

const char* genlua_force(force_id force) {
    switch (force) {
        case VIS_DEFAULT_FORCE: return "Vis.DEFAULT_FORCE";
        case VIS_FORCE_FRICTION: return "Vis.FORCE_FRICTION";
        case VIS_FORCE_GRAVITY: return "Vis.FORCE_GRAVITY";
        case VIS_NFORCES: return "Vis.NFORCES";
        default: return "nil";
    }
}

const char* genlua_limit(limit_id limit) {
    switch (limit) {
        case VIS_DEFAULT_LIMIT: return "Vis.DEFAULT_LIMIT";
        case VIS_LIMIT_BOX: return "Vis.LIMIT_BOX";
        case VIS_LIMIT_SPRINGBOX: return "Vis.LIMIT_SPRINGBOX";
        case VIS_NLIMITS: return "Vis.NLIMITS";
        default: return "nil";
    }
}

const char* genlua_blender(blend_id blender) {
    switch (blender) {
        case VIS_BLEND_NONE: return "Vis.BLEND_NONE";
        /* case VIS_DEFAULT_BLEND: */
        case VIS_BLEND_LINEAR: return "Vis.BLEND_LINEAR";
        case VIS_BLEND_PARABOLIC: return "Vis.BLEND_PARABOLIC";
        case VIS_BLEND_QUADRATIC: return "Vis.BLEND_QUADRATIC";
        case VIS_BLEND_SINE: return "Vis.BLEND_SINE";
        case VIS_BLEND_NEGGAMMA: return "Vis.BLEND_NEGGAMMA";
        case VIS_BLEND_EASING: return "Vis.BLEND_EASING";
        case VIS_NBLENDS: return "Vis.NBLENDS";
        default: return "nil";
    }
}

