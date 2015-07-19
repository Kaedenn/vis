
#include "genlua.h"
#include "kstring.h"
#include "types.h"
#include "emit.h"
#include "defines.h"
#include "drawer.h"

char* genlua_emit(emit_desc* emit, fnum when) {
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

const char* genlua_mutate(mutate_id fnid) {
    switch (fnid) {
        case VIS_MUTATE_PUSH: return "Vis.Vis.MUTATE_PUSH";
        case VIS_MUTATE_PUSH_DX: return "Vis.Vis.MUTATE_PUSH_DX";
        case VIS_MUTATE_PUSH_DY: return "Vis.Vis.MUTATE_PUSH_DY";
        case VIS_MUTATE_SLOW: return "Vis.Vis.MUTATE_SLOW";
        case VIS_MUTATE_SHRINK: return "Vis.Vis.MUTATE_SHRINK";
        case VIS_MUTATE_GROW: return "Vis.Vis.MUTATE_GROW";
        case VIS_MUTATE_AGE: return "Vis.Vis.MUTATE_AGE";
        case VIS_MUTATE_OPACITY: return "Vis.Vis.MUTATE_OPACITY";
        case VIS_MUTATE_SET_DX: return "Vis.Vis.MUTATE_SET_DX";
        case VIS_MUTATE_SET_DY: return "Vis.Vis.MUTATE_SET_DY";
        case VIS_MUTATE_SET_RADIUS: return "Vis.Vis.MUTATE_SET_RADIUS";
        case VIS_MUTATE_TAG_SET: return "Vis.Vis.MUTATE_TAG_SET";
        case VIS_MUTATE_TAG_INC: return "Vis.Vis.MUTATE_TAG_INC";
        case VIS_MUTATE_TAG_DEC: return "Vis.Vis.MUTATE_TAG_DEC";
        case VIS_MUTATE_TAG_ADD: return "Vis.Vis.MUTATE_TAG_ADD";
        case VIS_MUTATE_TAG_SUB: return "Vis.Vis.MUTATE_TAG_SUB";
        case VIS_MUTATE_TAG_MUL: return "Vis.Vis.MUTATE_TAG_MUL";
        case VIS_MUTATE_TAG_DIV: return "Vis.Vis.MUTATE_TAG_DIV";
        case VIS_MUTATE_PUSH_IF: return "Vis.Vis.MUTATE_PUSH_IF";
        case VIS_MUTATE_PUSH_DX_IF: return "Vis.Vis.MUTATE_PUSH_DX_IF";
        case VIS_MUTATE_PUSH_DY_IF: return "Vis.Vis.MUTATE_PUSH_DY_IF";
        case VIS_MUTATE_SLOW_IF: return "Vis.Vis.MUTATE_SLOW_IF";
        case VIS_MUTATE_SHRINK_IF: return "Vis.Vis.MUTATE_SHRINK_IF";
        case VIS_MUTATE_GROW_IF: return "Vis.Vis.MUTATE_GROW_IF";
        case VIS_MUTATE_AGE_IF: return "Vis.Vis.MUTATE_AGE_IF";
        case VIS_MUTATE_OPACITY_IF: return "Vis.Vis.MUTATE_OPACITY_IF";
        case VIS_MUTATE_SET_DX_IF: return "Vis.Vis.MUTATE_SET_DX_IF";
        case VIS_MUTATE_SET_DY_IF: return "Vis.Vis.MUTATE_SET_DY_IF";
        case VIS_MUTATE_SET_RADIUS_IF: return "Vis.Vis.MUTATE_SET_RADIUS_IF";
        case VIS_NMUTATES: return "Vis.Vis.NMUTATES";
        default: return "nil";
    }
}

const char* genlua_mutate_cond(mutate_cond_id id) {
    switch (id) {
        case VIS_MUTATE_IF_TRUE: return "Vis.MUTATE_IF_TRUE";
        case VIS_MUTATE_IF_EQ: return "Vis.MUTATE_IF_EQ";
        case VIS_MUTATE_IF_NE: return "Vis.MUTATE_IF_NE";
        case VIS_MUTATE_IF_LT: return "Vis.MUTATE_IF_LT";
        case VIS_MUTATE_IF_LE: return "Vis.MUTATE_IF_LE";
        case VIS_MUTATE_IF_GT: return "Vis.MUTATE_IF_GT";
        case VIS_MUTATE_IF_GE: return "Vis.MUTATE_IF_GE";
        case VIS_MUTATE_IF_EVEN: return "Vis.MUTATE_IF_EVEN";
        case VIS_MUTATE_IF_ODD: return "Vis.MUTATE_IF_ODD";
        default: return "nil";
    }
}

