
# The Vis.mutate and Vis.mutateif functions

These functions allow you to perform various transformations on existing particles.

## <code>Vis.mutate</code>

This function allows you to modify certain properties for each particle. These modifications are unconditional and are applied to every particle currently alive.

### Normal mutation

```lua
Vis.mutate(Vis.flist, when, mutate_func_id, factor1[, factor2, offset1, offset2])
```

Required parameters: `Vis.flist`, `when` (in milliseconds), `mutate_func_id`, and `factor1`. The remaining parameters default to 0 if not specified.

The following mutates are available:

* `Vis.MUTATE_PUSH` - `particle->dx *= factor1` and `particle->dy *= factor1`
* `Vis.MUTATE_PUSH_DX` - `particle->dx *= factor1`
* `Vis.MUTATE_PUSH_DY` - `particle->dy *= factor1`
* `Vis.MUTATE_SLOW` - `particle->dx /= factor1` and `particle->dy /= factor1`
* `Vis.MUTATE_SHRINK` - `particle->radius /= factor1`
* `Vis.MUTATE_GROW` - `particle->radius *= factor1`
* `Vis.MUTATE_AGE` - `particle->life = particle->lifetime * factor1`
* `Vis.MUTATE_OPACITY` - `particle->alpha = factor1`
* `Vis.MUTATE_SET_DX` - `particle->dx = factor1`
* `Vis.MUTATE_SET_DY` - `particle->dy = factor1`
* `Vis.MUTATE_SET_RADIUS` - `particle->radius = factor1`

### Tag mutation

Every particle has its own `tag`, or 32-bit number. These are set via the `tag` field in the emit table. These mutate functions exist to adjust the tag for all active particles. For conditional tag modification, see `Vis.MUTATE_TAG_SET_IF` <a href="#vismutateif">below</a>.

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

## <code>Vis.mutateif</code>

These functions allow you to modify certain particle properties only if the specified condition holds.

<!-- FIXME: mutate_set_dx, dy, radius, use both factor1 and factor2, which
breaks conditional mutation (as factor2 is checked) -->

```lua
Vis.mutateif(Vis.flist, when, ...)
```

The following conditional mutates are available:

* `Vis.MUTATE_TAG_SET_IF`
* `Vis.MUTATE_PUSH_IF`
* `Vis.MUTATE_PUSH_DX_IF`
* `Vis.MUTATE_PUSH_DY_IF`
* `Vis.MUTATE_SLOW_IF`
* `Vis.MUTATE_SHRINK_IF`
* `Vis.MUTATE_GROW_IF`
* `Vis.MUTATE_AGE_IF`
* `Vis.MUTATE_OPACITY_IF`
* `Vis.MUTATE_SET_DX_IF`
* `Vis.MUTATE_SET_DY_IF`
* `Vis.MUTATE_SET_RADIUS_IF`

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


