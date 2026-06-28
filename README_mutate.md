
# <code>Vis.mutate</code>

This function allows you to modify certain properties for each particle. These
modifications are unconditional and are applied to every particle currently
alive.

The `factor`, `check` and `offset` parameters can be either numbers or arrays
of two numbers. If the parameter is a number, then the second value will default
to zero.

```lua
Vis.mutate{
    Vis.flist,
    when,                                     -- When to apply the mutation event (in milliseconds)
    [func=]Vis.MUTATE_<func>,                 -- Which mutation event to use
    cond=Vis.MUTATE_IF_<cond>,                -- Only when using cond=Vis.MUTATE_IF_<cond>
    tag=<check-tag>,                          -- Only when using cond=Vis.MUTATE_IF_<tag-cond>
    newtag=<new-tag>,                         -- Only when using cond=Vis.MUTATE_TAG_<event>
    factor=<number-or-array-of-two-numbers>,  -- Amount to mutate by
    check=<number-or-array-of-two-numbers>,   -- Used with Vis.MUTATE_IF_{NEAR/FAR}
    offset=<number-or-array-of-two-numbers>   -- Used with Vis.MUTATE_IF_{ABOVE/BELOW/LEFT/RIGHT}
}
```

## Mutation Event Types

The following mutation events are available.

### Unconditional Mutation Events

For all these events, negative values for `factor` are permitted.

* `Vis.MUTATE_PUSH` - `particle->dx *= factor[1]` and `particle->dy *= factor[1]`
* `Vis.MUTATE_PUSH_DX` - `particle->dx *= factor[1]`
* `Vis.MUTATE_PUSH_DY` - `particle->dy *= factor[1]`
* `Vis.MUTATE_PUSH_DZ` - `particle->dz *= factor[1]`
* `Vis.MUTATE_SLOW` - `particle->dx /= factor[1]` and `particle->dy /= factor[1]`
* `Vis.MUTATE_SHRINK` - `particle->radius /= factor[1]`
* `Vis.MUTATE_GROW` - `particle->radius *= factor[1]`
* `Vis.MUTATE_AGE` - `particle->life = particle->lifetime * factor[1]`
* `Vis.MUTATE_OPACITY` - `particle->alpha = factor[1]`
* `Vis.MUTATE_SET_DX` - `particle->dx = random(factor[1] ± factor[2])`
* `Vis.MUTATE_SET_DY` - `particle->dy = random(factor[1] ± factor[2])`
* `Vis.MUTATE_SET_DZ` - `particle->dz = random(factor[1] ± factor[2])`
* `Vis.MUTATE_SET_RADIUS` - `particle->radius = random(factor[1] ± factor[2])`
* `Vis.MUTATE_SET_VERTICES` - `particle->vertices = random(factor[1] ± factor[2])`
* `Vis.MUTATE_SET_ANGLE` - `particle->angle = random(factor[1] ± factor[2])`
* `Vis.MUTATE_SET_FRICTION` - `particle->friction_coeff = factor[1]`
* `Vis.MUTATE_SET_GRAVITY` - `particle->gravity_coeff = factor[1]`

### Tag Mutation Events
* `Vis.MUTATE_TAG_SET` - `particle->tag.l = newtag`
* `Vis.MUTATE_TAG_INC` - `particle->tag.l += 1`
* `Vis.MUTATE_TAG_DEC` - `particle->tag.l -= 1`
* `Vis.MUTATE_TAG_ADD` - `particle->tag.l += newtag`
* `Vis.MUTATE_TAG_SUB` - `particle->tag.l -= newtag`
* `Vis.MUTATE_TAG_MUL` - `particle->tag.l *= newtag`
* `Vis.MUTATE_TAG_DIV` - `particle->tag.l /= newtag`

### Conditional Mutation Events

These mutation events apply only if the specified condition is met. The mutation
effects are identical to their unconditional counterparts.

* `Vis.MUTATE_PUSH_IF`
* `Vis.MUTATE_PUSH_DX_IF`
* `Vis.MUTATE_PUSH_DY_IF`
* `Vis.MUTATE_PUSH_DZ_IF`
* `Vis.MUTATE_SLOW_IF`
* `Vis.MUTATE_SHRINK_IF`
* `Vis.MUTATE_GROW_IF`
* `Vis.MUTATE_AGE_IF`
* `Vis.MUTATE_OPACITY_IF`
* `Vis.MUTATE_SET_DX_IF`
* `Vis.MUTATE_SET_DY_IF`
* `Vis.MUTATE_SET_DZ_IF`
* `Vis.MUTATE_SET_RADIUS_IF`
* `Vis.MUTATE_SET_VERTICES_IF`
* `Vis.MUTATE_SET_ANGLE_IF`
* `Vis.MUTATE_SET_FRICTION_IF`
* `Vis.MUTATE_SET_GRAVITY_IF`

### Conditional Tag Mutation Event
* `Vis.MUTATE_TAG_SET_IF` - `particle->tag = newtag` if condition is met

## Mutate Conditions

This is the powerful mechanism that applies the above mutation events only if
the particle meets the specified condition. When using a conditional mutate,
you **must** provide a `cond` parameter.

The following conditions are available:

* `Vis.MUTATE_IF_TRUE` - always true
* `Vis.MUTATE_IF_EQ` - `particle->tag == tag`
* `Vis.MUTATE_IF_NE` - `particle->tag != tag`
* `Vis.MUTATE_IF_LT` - `particle->tag < tag`
* `Vis.MUTATE_IF_LE` - `particle->tag <= tag`
* `Vis.MUTATE_IF_GT` - `particle->tag > tag`
* `Vis.MUTATE_IF_GE` - `particle->tag >= tag`
* `Vis.MUTATE_IF_EVEN` - `particle->tag % 2 == 0`
* `Vis.MUTATE_IF_ODD` - `particle->tag % 2 == 1`
* `Vis.MUTATE_IF_ABOVE` - `particle->y > offset[2]`
* `Vis.MUTATE_IF_BELOW` - `particle->y < offset[2]`
* `Vis.MUTATE_IF_LEFT` - `particle->x < offset[1]`
* `Vis.MUTATE_IF_RIGHT` - `particle->x > offset[1]`
* `Vis.MUTATE_IF_NEAR` - `dist(particle.xy, offset[1,2]) <= check[1]`
* `Vis.MUTATE_IF_FAR` - `dist(particle.xy, offset[1,2]) >= check[1]`

# Examples

## Direct Mutation

## Conditional Mutation

## Conditional Mutation Using Tags

# Old API

There are presently two competing APIs for `Vis.mutate`. The arguments for the
new API are generally the same as the old API. However, the old API uses
positional arguments whereas the new API uses keyword arguments. The old API is
kept around for legacy reasons, but the new API is recommended.

## Overloads

* `Vis.mutate(Vis.flist, when, func, factor1, [factor2, [offset1, [offset2]]])`
  * **Trigger:** `func` is an unconditional mutator (e.g., `Vis.MUTATE_PUSH`, `Vis.MUTATE_SLOW`, `Vis.MUTATE_SET_DX`).

* `Vis.mutate(Vis.flist, when, func, [newtag])`
  * **Trigger:** `func` is an unconditional tag mutator (e.g., `Vis.MUTATE_TAG_SET`, `Vis.MUTATE_TAG_INC`).

* `Vis.mutate(Vis.flist, when, func, cond, tag, newtag, [factor1, [factor2, [check_factor1, [check_factor2, [offset1, [offset2]]]]]])`
  * **Trigger:** `func` is `Vis.MUTATE_TAG_SET_IF` and `cond` is a tag-checking condition (`Vis.MUTATE_IF_TRUE` up to `Vis.MUTATE_IF_GE`).

* `Vis.mutate(Vis.flist, when, func, cond, tag, [factor1, [factor2, [check_factor1, [check_factor2, [offset1, [offset2]]]]]])`
  * **Trigger:** `func` is a standard conditional mutator (e.g., `Vis.MUTATE_PUSH_IF`) and `cond` is a tag-checking condition (`Vis.MUTATE_IF_TRUE` up to `Vis.MUTATE_IF_GE`).

* `Vis.mutate(Vis.flist, when, func, cond, newtag, [factor1, [factor2, [check_factor1, [check_factor2, [offset1, [offset2]]]]]])`
  * **Trigger:** `func` is `Vis.MUTATE_TAG_SET_IF` and `cond` is a non-tag condition (e.g., spatial checks like `Vis.MUTATE_IF_NEAR` or parity checks like `Vis.MUTATE_IF_EVEN`).

* `Vis.mutate(Vis.flist, when, func, cond, [factor1, [factor2, [check_factor1, [check_factor2, [offset1, [offset2]]]]]])`
  * **Trigger:** `func` is a standard conditional mutator (e.g., `Vis.MUTATE_PUSH_IF`) and `cond` is a non-tag condition (e.g., spatial checks like `Vis.MUTATE_IF_NEAR` or parity checks like `Vis.MUTATE_IF_EVEN`).

<!--
### Normal mutation

```lua
Vis.mutate(Vis.flist, when, mutate_func_id, factor1[, factor2, offset1, offset2])
```

Required parameters: `Vis.flist`, `when` (in milliseconds), `mutate_func_id`, and `factor1`. The remaining parameters default to 0 if not specified.

The following mutates are available:

* `Vis.MUTATE_PUSH` - `particle->dx *= factor1` and `particle->dy *= factor1`
* `Vis.MUTATE_PUSH_DX` - `particle->dx *= factor1`
* `Vis.MUTATE_PUSH_DY` - `particle->dy *= factor1`
* `Vis.MUTATE_PUSH_DZ` - `particle->dz *= factor1`
* `Vis.MUTATE_SLOW` - `particle->dx /= factor1` and `particle->dy /= factor1`
* `Vis.MUTATE_SHRINK` - `particle->radius /= factor1`
* `Vis.MUTATE_GROW` - `particle->radius *= factor1`
* `Vis.MUTATE_AGE` - `particle->life = particle->lifetime * factor1`
* `Vis.MUTATE_OPACITY` - `particle->alpha = factor1`
* `Vis.MUTATE_SET_DX` - `particle->dx = factor1`
* `Vis.MUTATE_SET_DY` - `particle->dy = factor1`
* `Vis.MUTATE_SET_DZ` - `particle->dz = factor1`
* `Vis.MUTATE_SET_RADIUS` - `particle->radius = factor1`
* `Vis.MUTATE_SET_VERTICES` - `particle->vertices = factor1`
* `Vis.MUTATE_SET_ANGLE` - `particle->angle = factor1`

### Tag mutation

Every particle has its own `tag`, or 32-bit number. These are set via the `tag` field in the emit table. These mutate functions exist to adjust the tag for all active particles. For conditional tag modification, see `Vis.MUTATE_TAG_SET_IF` below.

```lua
Vis.mutate(Vis.flist, when, mutate_tag_func_id[, tag])
```

Required parameters: `Vis.flist`, `when` (in milliseconds), and `mutate_tag_func_id`. `tag` defaults to 0 if not specified.

The following tag mutates are available:

* `Vis.MUTATE_TAG_SET` - `particle->tag = tag`
* `Vis.MUTATE_TAG_INC` - `particle->tag += 1`
* `Vis.MUTATE_TAG_DEC` - `particle->tag -= 1`
* `Vis.MUTATE_TAG_ADD` - `particle->tag += tag`
* `Vis.MUTATE_TAG_SUB` - `particle->tag -= tag`
* `Vis.MUTATE_TAG_MUL` - `particle->tag *= tag`
* `Vis.MUTATE_TAG_DIV` - `particle->tag /= tag`

### Conditional mutation

These functions allow you to modify certain particle properties only if the specified condition holds.

```lua
Vis.mutate(Vis.flist, when, mutate_func_id, cond,
      tag,                        -- only if cond is EQ, NE, LT, LE, GT, GE
      newtag,                     -- only if mutate_func_id is Vis.MUTATE_TAG_SET_IF
      factor1, factor2,           -- assigned
      checkfactor1, checkfactor2, -- checked
      offset1, offset2)           -- compared
```

The following conditional mutates are available:

* `Vis.MUTATE_TAG_SET_IF`
* `Vis.MUTATE_PUSH_IF`
* `Vis.MUTATE_PUSH_DX_IF`
* `Vis.MUTATE_PUSH_DY_IF`
* `Vis.MUTATE_PUSH_DZ_IF`
* `Vis.MUTATE_SLOW_IF`
* `Vis.MUTATE_SHRINK_IF`
* `Vis.MUTATE_GROW_IF`
* `Vis.MUTATE_AGE_IF`
* `Vis.MUTATE_OPACITY_IF`
* `Vis.MUTATE_SET_DX_IF`
* `Vis.MUTATE_SET_DY_IF`
* `Vis.MUTATE_SET_DZ_IF`
* `Vis.MUTATE_SET_RADIUS_IF`
* `Vis.MUTATE_SET_VERTICES_IF`
* `Vis.MUTATE_SET_ANGLE_IF`

The following mutate conditions are available:

* `Vis.MUTATE_IF_TRUE`
* `Vis.MUTATE_IF_EQ`
* `Vis.MUTATE_IF_NE`
* `Vis.MUTATE_IF_LT`
* `Vis.MUTATE_IF_LE`
* `Vis.MUTATE_IF_GT`
* `Vis.MUTATE_IF_GE`
* `Vis.MUTATE_IF_EVEN`
* `Vis.MUTATE_IF_ODD`
* `Vis.MUTATE_IF_ABOVE`
* `Vis.MUTATE_IF_BELOW`
* `Vis.MUTATE_IF_LEFT`
* `Vis.MUTATE_IF_RIGHT`
* `Vis.MUTATE_IF_NEAR`
* `Vis.MUTATE_IF_FAR`


-->