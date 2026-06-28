
# Hand-Crafted Particle Visualization to Music, via OpenGL and Lua

## Table of Contents

- [Background information](#background-information)
- [Compiling this thing](#compiling-this-thing)
  - [Make targets:](#make-targets)
  - [Dependencies:](#dependencies)
- [Running this thing](#running-this-thing)
  - [Controls](#controls)
  - [Other invocations](#other-invocations)
    - [Click-and-drag method](#click-and-drag-method)
    - [Script mode](#script-mode)
  - [Writing your own scripts](#writing-your-own-scripts)
  - [A note about uncertainty:](#a-note-about-uncertainty)
- [Lua API Documentation](#lua-api-documentation)
  - [`module Vis = require("Vis")` - The Vis Module](#vis-module)
  - [`module VisUtil = require("visutil")` - The Vis Utility Module](#visutil-module)
  - [The Emit Table](#the-emit-table)
  - [`module Emit = require("emit")` - The Emit Module](#emit-module)
  - [`Vis.mutate`](#vis-mutate)
    - [Mutation Event Types](#mutation-event-types)
      - [Unconditional Mutation Events](#unconditional-mutation-events)
      - [Tag Mutation Events](#tag-mutation-events)
      - [Conditional Mutation Events](#conditional-mutation-events)
      - [Conditional Tag Mutation Event](#conditional-tag-mutation-event)
    - [Mutate Conditions](#mutate-conditions)
  - [`Vis.mutate` Examples](#vis-mutate-examples)
    - [Direct Mutation](#direct-mutation)
    - [Conditional Mutation](#conditional-mutation)
    - [Conditional Mutation Using Tags](#conditional-mutation-using-tags)
  - [`Vis.mutate` Old API](#vis-mutate-old-api)
    - [Overloads](#overloads)
      - [Normal mutation](#normal-mutation)
      - [Tag mutation](#tag-mutation)
      - [Conditional mutation](#conditional-mutation)
  - [`module Message = require("message")` - The Message Module](#message-module)
- [Planned Features](#planned-features)
- [Credits](#credits)

## Background information

This project was originally created to investigate the question, "can you
simulate synesthesia for someone who does not have synesthesia?"

Synesthesia, specifically chromesthesia, is the perception of shapes when
listening to music. The exact shapes, their positions, colors, and behaviors
are unique to the individual. Therefore, this project does not attempt to
simulate synesthesia in general, but rather simulate a specific person's
experience.

Chromesthesia has been described as "a fireworks display for the mind" and can
be genuinely enjoyable.

Unfortunately, I do not have chromesthesia. However, I do have a form of
"music-location-shape" synesthesia where music triggers shapes at various
locations in my mind, but without color information.

<details>
<summary><strong>Rules of Chromesthesia</strong></summary>

To ensure the experience I create closely matches real chromesthesia, I have
decided to abide by the following two arbitrary rules:

1.  **Duration**: Shapes should only appear on-screen for the duration of
    the note, instrument, chord, or lyric. Once the sound stops or changes,
    the shape should cease or change.
2.  **Consistency**: The same sound on the same instrument should give the
    same shape regardless of when it's played.

</details>

## Compiling this thing

This project depends on OpenGL and Lua. There are several `$(MAKE)` targets,
including `all`, `debug`, `fast`, `valgrind`, etc. A simple `make all` should
do the trick just fine. If the program seems slow, try `make fast`.

### Make targets:

* `all` - Equivalent to `make debug`.
* `fast` - Enable fierce optimizations and link-time optimization.
* `release` - Equivalent to `make fast`.
* `debug` - Enable debug output.
* `trace` - Enable _very noisy_ debug output.
* `profile` - Compile with `make fast` and profiling flags, invoke
  `lua/demos/demo_5_random.lua`, and run `gprof` on the generated output.
* `valgrind` - Compile with `make debug` and invoke `valgrind` on
  `lua/demos/demo_4_random.lua`.
* `leakcheck` - Invoke `valgrind` with `--leak-check=full`.
* `leakcheck-reachable` - `valgrind` with full check and reachable info.
* `clean` - Removes all generated and intermediate files.
* `distclean` - `make clean` and remove generated frames (`$(FP_BASE)_*.png`).

### Dependencies:

* LuaJIT 2.1 (5.1 compat) (with development libraries)
* OpenGL 4.3, GLFW, GLEW (for core functionality)
* clang (optional, for luautf8.so)
* libjson-c (optional, for config.json)
* libpulse (optional, Linux only, for audio playback matching)
* libfreetype2 (optional, for text)
* libreadline (optional, for interactive commands)

## Running this thing

This program has just one argument for intended usage: `-l <FILE>`. This
argument loads the Lua script `<FILE>` and executes it. For information on
writing scripts, see any of the Lua files in the `test` directory, the `lua`
directory, or the section `Writing your own scripts` below.

### Controls

In addition to anything the Lua scripts add, the following keyboard and mouse
inputs are available while the program is running:

Pressing `^D` (`Control+D`) on the command-line will close the program.

Pressing `ESC` will close the program.

Pressing `Space` will pause the program. Pressing it again will resume the
program.

Clicking anywhere on the screen will trigger the default emit rule. Moving the
cursor while holding down the left mouse button will repeatedly trigger the
default emit rule. This rule can be changed via `Vis.settrace`.

### Other invocations

There are two main ways to run this thing:

1.  With no arguments, using click-and-drag
2.  With the `-l` argument and a path to a `.lua` script

#### Click-and-drag method

First, run the program with no arguments. Then, click and hold with the left
mouse button and move the mouse around the screen. This will cause small circles
of particles to appear. It is possible to change the parameters of this
emission, but that requires a script file (the second method).

The size of the particles can be changed using the scroll wheel.

#### Script mode

This is the most recommended way to run the program. I recommend running the
following script:

```sh
$ ./vis -l test/4_random_long.lua
```

This gives you a good taste of what the particle engine can do, although it
leaves out a good number of details.

The full documentation of script mode is below:

### Writing your own scripts

This is the fun part. There are three modules available: Vis, VisUtil, and Emit.

### A note about uncertainty:

The vast majority of values have an uncertainty counterpart, which is used to
adjust the particle emission. For example, the uncertainty in horizontal
position `x` is denoted as `ux`, and every particle emitted by this
table will be positioned at a random spot between `x - ux` and
`x + ux`.  Position, radius, angle, life, and color all have uncertainty
values which all follow this rule.

## Lua API Documentation

### <a name="vis-module"></a>`module Vis = require("Vis")` - The Vis Module

This is the core library. Its fields are:

`userdata Vis.flist`: The `frame list` structure. In order to schedule
emits, the `vis` executable needs to know where the scheduling is stored.
This is that location.

`userdata Vis.script`: This field is present in order to perform advanced
operations, such as scheduling callbacks (strings of Lua script to be executed
after a certain amount of time has passed).

`function Vis.debug(...)`: For debugging purposes, this function prints
out its arguments to the debugging stream if and only if the program was
compiled with debugging enabled (via `make debug`).

`function Vis.debugp(string...)`: For debugging purposes, this function prints
out its arguments as a single concatenated string. Note that passing anything
other than strings may result in errors.

`function Vis.emit(...)`: Core emit function. Using this is discouraged unless
performance is a massive concern. For alternatives, see
`VisUtil.make_emit_table`, `VisUtil.emit_table`, the `Emit` class, as well as
the documentation on emit tables. This function actually performs the particle
logic.

`function Vis.audio(path)`: Load the audio file given by `path`.

`function Vis.play()`: Plays the current audio file.

`function Vis.pause()`: Pauses the current audio file.

`function Vis.volume(float)`: Adjust the volume; values are between 0 and 1.

`function Vis.seek(hundreths-of-a-second)`: Seeks the current audio file
to the offset given by `hundredths-of-a-second`.

`function Vis.seekms(Vis.flist, when, milliseconds)`: Sets the current
position of the schedule to `milliseconds` when `when` milliseconds
have passed.

`function Vis.seekframe(Vis.flist, when, frame_number)`: Sets the current
position of the schedule to `frame_number` after `when` milliseconds
have passed. See `Vis.frames2msec` and `Vis.msec2frames` to convert
between frames and milliseconds.

`function Vis.audiosync(Vis.flist, when, frame_count)`: At the specific frame,
stop processing visual effects, mute the audio, pause the audio, delay for the
specified number of frames, then un-mute and restart the audio track at the
beginning (0ms). This is needed when the audio track does not start playing
immediately and ensures both the audio and visual effects have enough time to
synchronize.

`function Vis.delay(Vis.flist, when, frame_count)`: Delays the visual effects
for the specified number of frames, while allowing the audio to continue. Use
this if the audio takes longer to load or has "dead time" before the song
begins.

`function Vis.bgcolor(Vis.script, r, g, b)`: Sets the background color to
the values given. Each value is between 0 and 1.

`function Vis.rotate(Vis.flist, when, theta, phi)`: Schedules a global camera
rotation around the Y (`theta`) and X (`phi`) axes after `when` milliseconds
have passed.

`function Vis.mutate(Vis.flist, when, func, ...)`: Schedules the
mutation given by `func` (see constants `Vis.MUTATE_*`) after `when`
milliseconds have passed. Additional parameters are passed to the
function as needed. Mutates affect all living particles on the screen.

`function Vis.callback(Vis.flist, when, Vis.script, lua_script)`: Executes
the string `lua_script` in the same context as the calling file after `when`
milliseconds have passed.

`function Vis.callback(Vis.flist, when, Vis.script, func, ...args)`: Executes
the Lua function `func` with the provided `...args` after `when` milliseconds
have passed.

`function Vis.fps()`: Returns the current derived frames per second, which
should be fairly close to `Vis.FPS_LIMIT`.

`function Vis.settrace(Vis.script, emit_table)`: Change what happens when the
user clicks and drags on the screen. The `emit_table` can be a native Lua
`table` or an `Emit` instance.

`function Vis.frames2msec(frame_number)`: Converts argument `frame_number` to
milliseconds.

`function Vis.msec2frames(milliseconds)`: Converts argument `milliseconds` to
a frame number.

`function Vis.exit()`: Terminates the application immediately.

`function Vis.exit(Vis.flist, when)`: Schedules the application to terminate
after `when` milliseconds have passed.

`function Vis.gotoframe(Vis.flist, whereto)`: Immediately changes the
application's current frame state to `whereto`.

`function Vis.get_audio_delay()`: Returns the audio delay offset in milliseconds.
Note that this relies on the latency measurement system, which currently only
works if `libpulse` support is present. If it is not present, this function
is a no-op and will return 0.

`function Vis.emitnow(Vis.script, ...)`: Immediately executes an emission
using the provided arguments, bypassing the scheduler. Arguments are identical
to those of `Vis.emit()`.

`function Vis.get_debug(Vis.script, what)`: Returns internal debug values based
on the string `what`. The supported strings and their return values are:
* `"PARTICLES-EMITTED"`: Total particles emitted so far.
* `"PARTICLE-COUNT"`: The current number of active/living particles.
* `"TIME-NOW"`: The current internal elapsed time in milliseconds.
* `"FRAMES-EMITTED"`: Total number of scheduled frames emitted.
* `"NUM-MUTATES"`: The number of scheduled mutates processed.
* `"PARTICLES-MUTATED"`: The total number of times particles have been mutated.
* `"PARTICLE-TAGS-MODIFIED"`: The total number of particle tag modifications applied.
* `"FRAME-EMIT-COUNTS"`: A table containing the count for each emitted frame type.

`function Vis.dump_particles(Vis.script, path, [mode], [header])`: Dumps all
active particles to the file specified by `path` in CSV format. The `mode`
defaults to `"w"`. If `header` is `true`, a row containing the column names is
output first. Returns the number of particles written.

`function Vis.dump_particles(Vis.script, file_handle, [header])`: Dumps all
active particles to the open Lua `file_handle` in CSV format. If `header` is
`true`, a row containing the column names is output first. Returns the number
of particles written.

`constant Vis.FPS_LIMIT`: The intended frames-per-second at which this
program runs. At the time of writing, this is set to 30.

`constant Vis.WIDTH`: The width of the program window. Currently set to 800
but can be changed on the command-line.

`constant Vis.HEIGHT`: The height of the program window. Currently set to 600
but can be changed on the command-line.

`constant Vis.DEFAULT_BLEND`: Equal to `Vis.BLEND_NONE`.

`constant Vis.BLEND_NONE`: Particles do not fade out; they simply vanish
suddenly once their lifetime runs out.

`constant Vis.BLEND_LINEAR`: Particles fade out at a constant rate during
their lifetime.

`constant Vis.BLEND_QUADRATIC`: Particles start fading slowly and fade
faster as they age.

`constant Vis.BLEND_NEGGAMMA`: Particles fade almost suddenly at the end of
their lifetime.

`constant Vis.NBLENDS`: Equal to the number of possible blend effects.

`constant Vis.DEFAULT_FORCE`: Particles do not undergo any additional force
as they age.

`constant Vis.FORCE_FRICTION`: Particles gradually slow down as they age.

`constant Vis.FORCE_GRAVITY`: Particles gradually accelerate towards the
bottom of the screen as they age.

`constant Vis.NFORCES`: Equal to the number of possible force effects.

`constant Vis.DEFAULT_LIMIT`: Particles are free to move off of the screen.

`constant Vis.LIMIT_BOX`: Particles stop at the edges of the screen.

`constant Vis.LIMIT_SPRINGBOX`: Particles bounce off the edges of the
screen.

`constant Vis.NLIMITS`: Equal to the number of possible limit choices.

`constant Vis.MUTATE_PUSH`: Accelerates the particles by the coefficient
given.

`constant Vis.MUTATE_SLOW`: Decelerates the particles by the coefficient
given.

`constant Vis.MUTATE_SHRINK`: Shrinks the particles by the coefficient
given.

`constant Vis.MUTATE_GROW`: Grows the particles by the coefficient given.

`constant Vis.MUTATE_PUSH_DX`: Accelerates the particles in the horizontal
direction by the coefficient given.

`constant Vis.MUTATE_PUSH_DY`: Accelerates the particles in the vertical
direction by the coefficient given.

`constant Vis.MUTATE_PUSH_DZ`: Accelerates the particles in the Z direction
by the coefficient given.

`constant Vis.MUTATE_AGE`: Sets the age of the particles to the product of
their total lifetime and the coefficient given.

`constant Vis.MUTATE_OPACITY`: Sets the particle base opacity to the
coefficient given. The blending functions are multiplied by this value.

`constant Vis.MUTATE_SET_DX`: Sets the particles' horizontal velocity to the
coefficient given.

`constant Vis.MUTATE_SET_DY`: Sets the particles' vertical velocity to the
coefficient given.

`constant Vis.MUTATE_SET_DZ`: Sets the particles' Z velocity to the
coefficient given.

`constant Vis.MUTATE_SET_RADIUS`: Sets the particle radius.

`constant Vis.MUTATE_SET_VERTICES`: Sets the particle vertex count (e.g.
3 for triangle, 4 for square).

`constant Vis.MUTATE_SET_ANGLE`: Sets the particle rotation angle.

`constant Vis.MUTATE_SET_FRICTION`: Sets the particle friction.

`constant Vis.MUTATE_SET_GRAVITY`: Sets the particle gravity.

`constant Vis.MUTATE_TAG_SET`: Tags all particles by setting their tag
value to the integer given.

`constant Vis.MUTATE_TAG_INC`: Increases all particles' tags by one.

`constant Vis.MUTATE_TAG_DEC`: Decreases all particles' tags by one.

`constant Vis.MUTATE_TAG_ADD`: Adds the value given to all particles' tags.

`constant Vis.MUTATE_TAG_SUB`: Subtracts the value given from all particles'
tags.

`constant Vis.MUTATE_TAG_MUL`: Multiplies all particles' tags by the value
given.

`constant Vis.MUTATE_TAG_DIV`: Divides all particles' tags by the value given.

`constant Vis.MUTATE_TAG_SET_IF`: Apply `Vis.MUTATE_TAG_SET` if the condition
specified evaluates to true.

`constant Vis.MUTATE_PUSH_IF`: Apply `Vis.MUTATE_PUSH` if the particles' tag
and the tag value specified satisfies the condition specified (see
`Vis.MUTATE_IF_*` below)

`constant Vis.MUTATE_PUSH_DX_IF`: Apply `Vis.MUTATE_PUSH_DX` if the
condition specified evaluates to true against the particle and the tag given.

`constant Vis.MUTATE_PUSH_DY_IF`: As above, with `Vis.MUTATE_PUSH_DY`

`constant Vis.MUTATE_PUSH_DZ_IF`: As above, with `Vis.MUTATE_PUSH_DZ`

`constant Vis.MUTATE_SLOW_IF`: As above, with `Vis.MUTATE_SLOW`

`constant Vis.MUTATE_SHRINK_IF`: As above, with `Vis.MUTATE_SHRINK`

`constant Vis.MUTATE_GROW_IF`: As above, with `Vis.MUTATE_GROW`

`constant Vis.MUTATE_AGE_IF`: As above, with `Vis.MUTATE_AGE`

`constant Vis.MUTATE_OPACITY_IF`: As above, with `Vis.MUTATE_OPACITY`

`constant Vis.MUTATE_SET_DX_IF`: As above, with `Vis.MUTATE_SET_DX`

`constant Vis.MUTATE_SET_DY_IF`: As above, with `Vis.MUTATE_SET_DY`

`constant Vis.MUTATE_SET_DZ_IF`: As above, with `Vis.MUTATE_SET_DZ`

`constant Vis.MUTATE_SET_RADIUS_IF`: As above, with `Vis.MUTATE_SET_RADIUS`

`constant Vis.MUTATE_SET_VERTICES_IF`: As above, with `Vis.MUTATE_SET_VERTICES`

`constant Vis.MUTATE_SET_ANGLE_IF`: As above, with `Vis.MUTATE_SET_ANGLE`

`constant Vis.MUTATE_SET_FRICTION_IF`: As above, with `Vis.MUTATE_SET_FRICTION`

`constant Vis.MUTATE_SET_GRAVITY_IF`: As above, with `Vis.MUTATE_SET_GRAVITY`

`constant Vis.NMUTATES`: Equal to the number of possible mutate choices
listed above.

`constant Vis.MUTATE_IF_TRUE`: Satisfied for all particles regardless of
their tag value.

`constant Vis.MUTATE_IF_EQ`: Satisfied when the particle's tag equals the
tag given.

`constant Vis.MUTATE_IF_NE`: Satisfied when the particle's tag does not
equal the tag given.

`constant Vis.MUTATE_IF_LT`: Satisfied when the particle's tag is less
than the tag given.

`constant Vis.MUTATE_IF_LE`: Satisfied when the particle's tag is less
than or equal to the tag given.

`constant Vis.MUTATE_IF_GT`: Satisfied when the particle's tag is greater
that the tag given.

`constant Vis.MUTATE_IF_GE`: Satisfied when the particle's tag is greater
than or equal to the tag given.

`constant Vis.MUTATE_IF_EVEN`: Satisfied when the particle's tag is even.

`constant Vis.MUTATE_IF_ODD`: Satisfied when the particle's tag is odd.

`constant Vis.MUTATE_IF_ABOVE`: Satisfied when the particle's Y coordinate is
less than or equal to the condition parameter.

`constant Vis.MUTATE_IF_BELOW`: Satisfied when the particle's Y coordinate
is greater than or equal to the condition parameter.

`constant Vis.MUTATE_IF_LEFT`: Satisfied when the particle's X coordinate is
less than or equal to the condition parameter.

`constant Vis.MUTATE_IF_RIGHT`: Satisfied when the particle's X coordinate is
greater than or equal to the condition parameter.

`constant Vis.MUTATE_IF_NEAR`: Satisfied when the particle's distance to the
specified coordinate is less than or equal to the distance threshold.

`constant Vis.MUTATE_IF_FAR`: Satisfied when the particle's distance to the
specified coordinate is greater than or equal to the distance threshold.

`constant Vis.FORCE_FRICTION_COEFF`: The strength of friction, from 0 to 1.

`constant Vis.FORCE_GRAVITY_FACTOR`: The strength of gravity, from 0 to 1.

`constant Vis.NFRAMES`: The maximum number of frames that can be
scheduled. This is currently set equal to 15 minutes at 60 frames per
second (900,000).

`constant Vis.AUDIO_FREQ`: The frequency of audio playback, set to
44.1KHz.

`constant Vis.AUDIO_SAMPLES`: The number of samples per frame, set to 512.

`constant Vis.AUDIO_CHANNELS`: The number of channels, set to 2.

`constant Vis.CONST_PUSH_STOP`: Special number that, when passed with
`Vis.MUTATE_PUSH` or any other mutate function starting with
`MUTATE_PUSH`, stops all particles.

`constant Vis.CONST_AGE_BORN`: Special number that, when passed with
`Vis.MUTATE_AGE` or `Vis.MUTATE_AGE_IF`, resets all particles' ages.

`constant Vis.CONST_AGE_DEAD`: Special number that, instead of resetting
particle ages, causes particles to die by setting their age to the value
representing end-of-life.

`constant Vis.DEBUG`: The value of the `DEBUG` macro, set when the
program is compiled. It cannot be changed after compilation. It can be one of
the following values:

`constant Vis.DEBUG_NONE`: No debugging.

`constant Vis.DEBUG_VERBOSE`: A little debugging.

`constant Vis.DEBUG_DEBUG`: All debugging, other than really noisy stuff.

`constant Vis.DEBUG_INFO`: Include more noisy debug messages.

`constant Vis.DEBUG_TRACE`: Include all debug messages, regardless of
noise. This includes allocation and deallocation functions, for tracking down
memory leaks.

### <a name="visutil-module"></a>`module VisUtil = require("visutil")` - The Vis Utility Module

This module is pure Lua and resides in `lua/visutil.lua`. Have a look there
to see how everything is implemented.  The emit table generated via the
`VisUtil` functions has a large number of members, all of which are public
and freely modifiable.

`function VisUtil.make_emit_table()`: Returns a newly-constructed emit
table with the position set to the center of the screen, radius set to 1,
color set to white, and blend set to linear.

`function VisUtil.center_emit_table(table, x, y, ux, uy)`: Centers the emit
table `table` at `x` and `y`, with a variance of `ux` and
`uy`. With only one argument, the table is centered at the center of the
screen and the variances are set to zero. Negative values wrap around the
screen from the opposite side. For example, `-VIS_WIDTH/4 == 3*VIS_WIDTH/4`

`function VisUtil.color_emit_table(table, r, g, b, ur, ug, ub)`: Sets the
color to the values given, with variances given by `ur`, `ug`, and
`ub`. Colors can be either from 0 to 1, or from 0 to 255. Omitted values
are set to zero.

`function VisUtil.emit_table(table)`: Invokes `Vis.emit`, passing the
table given by `table`.

`function VisUtil.seek_to(milliseconds)`: Invokes both `Vis.seek()` and
`Vis.seekms` with the value given.

`function VisUtil.set_trace_table(table)`: Invokes `Vis.settrace`,
passing the table given. This is how you modify the click-and-drag emission
parameters.

### The Emit Table

The keys of a valid emit table and the fields wrapped by an `Emit`
instance are listed below.

`integer count`: The number of particles to emit.

`integer when`: The time (in milliseconds) at which to emit this frame.

`integers x, ux`: The horizontal position of emission,
    `0 <= x < Vis.WIDTH`. Negative values are allowed and wrap around from
the right side of the screen.

`integers y, uy`: The vertical position of emission,
    `0 <= y < Vis.HEIGHT`. Negative values are allowed and wrap around
from the bottom of the screen.

`floats radius, uradius`: The radius, in pixels, of the particles to
emit. As a special case, a particle with a radius of 1 will be a single pixel
unless argument `-e` is passed to `vis` on the command-line.

`floats dx, dy`: The base initial velocity of the particles to emit, measured in
pixels per frame. This is added to the radial velocity defined by `ds` and
`theta`.

`floats ds, uds`: The initial radial speed of the particles to emit, measured in
pixels per frame.

`floats theta, utheta`: The angle at which to emit the particles, in radians.

`integer depth`: Z-depth. This value affects both the draw order of the
particles and their location along the perspective-projection vector. This is
only visible when using `Vis.rotate(...)` to adjust the vector.

`integers life, ulife`: The lifetime, in milliseconds, of the particles
to emit.

`floats r, g, b, ur, ug, ub`: The color of the particles to
emit. It is recommended to use `VisUtil.color_emit_table` or
`Emit:color` over using these values directly. Values are between 0 and 1.

`enumeration force`: One of the `Vis.FORCE_` values. These values affect the
particles' velocities each frame. The default force is no force.

`enumeration limit`: One of the `Vis.LIMIT_` values. These values affect how
particles behave when approaching the edges of the screen.

`enumeration blender`: One of the `Vis.BLEND_` values. These control the
evolution of particle transparency over time. `Vis.BLEND_LINEAR` is the
default.

`float friction`: Friction coefficient, if `force` is `Vis.FORCE_FRICTION`.
Defaults to 0.99. The particle's velocity is multiplied by this value
each frame.

`float gravity`: Gravity coefficient, if `force` is `Vis.FORCE_GRAVITY`.
Defaults to 0.03. The particle's Y velocity is increased by this value
each frame.

`integer vertices`: Number of vertices to use for the particle.
Defaults to 4.

`float angle`: Rotation angle of the particle in radians. Defaults to 0.

`integer tag`: Tag to use for the particle. Defaults to 0.

### <a name="emit-module"></a>`module Emit = require("emit")` - The Emit Module

This module is also pure Lua and resides in `lua/emit.lua`. Have a look
there to see how it works. Its purpose is to lessen the strain of working with
emit tables directly.

`e = Emit:new()`, `e = Emit:new(other_emit_instance)`: Creates an
instance of the emit class, optionally copying from an existing one.

`e2 = e:clone()`, `e2 = e:copy()`: Clones the existing emit class
`e`. These are identical to `e2 = Emit:new(e)` and are provided for
convenience.

`e:emit()`: Schedules the emit at the offset configured via
`e:when()`. This is intended to replace `VisUtil.emit_table()`.

`e:emit_at(when)`: Convenience function which calls `e:when(when)` and
then `e:emit()`.

`e:emit_now()`: Bypasses the `flist` entirely and performs the emit
immediately. This is useful inside a callback function or on one of the
existing callbacks, `Vis.on_keydown`, `Vis.on_mousemove`, etc.

`e:set_trace()`: Intended to replace `VisUtil.set_trace_table()`;
please use this over the `VisUtil` function.

`e:str()`: Converts the emit table to a string, clearly describing its
current configuration.

`e:get(field)`: Returns the current value of the field specified, where
`field` is one of the fields in the listing above.

`e:set(field, value)`: Sets the value of the field given to the value
specified. Use this only when there are no other functions that perform the
operation you're trying to accomplish.

`e:count(n)`: Configure the number of particles to emit.

`e:when(t)`: Configure the time to emit, in milliseconds.

`e:center(x, y)`: Places the center of the emit at the position given.
Omitting `x` and `y` will place the emit at the center of the screen.

`e:radius(radius, uradius)`: Configure the emit's radius. The second
parameter can be omitted and defaults to zero.

`e:velocity(dx, dy)`: Configure the emit's base initial velocity. These values
are added to the radial velocity defined by `ds` and `theta`. Any omitted value
defaults to zero.

`e:ds(ds, uds)`: Configure the emit's initial radial speed. The second
parameter defaults to zero.

`e:theta(theta, utheta)`: Configure the emit's angle. The second parameter
defaults to zero. These angles are in radians.

`e:life(life, ulife)`: Configure the emit's lifetime to the value given,
in milliseconds. The second parameter defaults to zero.

`e:color(r, g, b, ur, ug, ub)`: Configure the emit's color to the values
given, which should be between 0 and 1. Any omitted value defaults to zero.

`e:color2(rgb)`: Configure the emit's color to the entries of array
`rgb`, which should have six total entries.

`e:color3(rgb, urgb)`: Configure the emit's color to the entries of both
arrays. The second array, if omitted, defaults to a sequence of three zeros.

`e:force(force)`: Configure the emit's force method to the value given.
Value must be one of the `Vis.FORCE_` constants.

`e:limit(limit)`: Configure the emit's limit method to the value given.
Value must be one of the `Vis.LIMIT_` constants.

`e:blender(blend)` Configure the emit's alpha-blending method to the
method given, which must be one of the `Vis.BLEND_` constants.

### <a name="vis-mutate"></a><code>Vis.mutate</code>

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

#### Mutation Event Types

The following mutation events are available.

##### Unconditional Mutation Events

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

##### Tag Mutation Events
* `Vis.MUTATE_TAG_SET` - `particle->tag.l = newtag`
* `Vis.MUTATE_TAG_INC` - `particle->tag.l += 1`
* `Vis.MUTATE_TAG_DEC` - `particle->tag.l -= 1`
* `Vis.MUTATE_TAG_ADD` - `particle->tag.l += newtag`
* `Vis.MUTATE_TAG_SUB` - `particle->tag.l -= newtag`
* `Vis.MUTATE_TAG_MUL` - `particle->tag.l *= newtag`
* `Vis.MUTATE_TAG_DIV` - `particle->tag.l /= newtag`

##### Conditional Mutation Events

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

##### Conditional Tag Mutation Event
* `Vis.MUTATE_TAG_SET_IF` - `particle->tag = newtag` if condition is met

#### Mutate Conditions

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

### <a name="vis-mutate-examples"></a><code>Vis.mutate</code> Examples

#### Direct Mutation

#### Conditional Mutation

#### Conditional Mutation Using Tags

### <a name="vis-mutate-old-api"></a><code>Vis.mutate</code> Old API

There are presently two competing APIs for `Vis.mutate`. The arguments for the
new API are generally the same as the old API. However, the old API uses
positional arguments whereas the new API uses keyword arguments. The old API is
kept around for legacy reasons, but the new API is recommended.

#### Overloads

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

##### Normal mutation

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

##### Tag mutation

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

##### Conditional mutation

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

### <a name="message-api"></a><code>module Message = require("message")</code>

This API allows you to display text on the screen using particle emits. Each letter is a grid of on/off bits, where the on bits are where an emit is made. This module requires having an `Emit` class instance.

```lua
local msg = Message:new{emit=<emit-instance>, line_spacing=<optional-line-spacing>, zoom=<optional-zoom-multiplier>}
```

> [!NOTE]
> The message instance does not make a copy of the emit object. Modifying the emit object will also modify the message's emit object (as they are the exact same object in memory). You can use this to your advantage to change the color or lifetime of text on the fly.

#### Methods

* `msg:emit_char(when, char, anchorx, anchory, letterx, lettery)`: Emit a single character at the given location. If `when` is `nil`, `e:emitnow()` is called instead of scheduling.
* `msg:emit_message(when, message, anchorx, anchory)`: Emit a sequence of letters (a string) starting at the given location.
* `msg:emit_center_message(when, message, anchorx, anchory)`: Emit a center-aligned string at the given location.
* `msg:emit_lines(when, lines, anchorx, anchory)`: Emit a table of strings on consecutive lines.
* `msg:emit_center_lines(when, lines, anchorx, anchory)`: Emit a table of strings on consecutive lines, center-aligned.
* `msg:emit_text(when, text, anchorx, anchory)`: Emit arbitrary text (which may contain multiple lines) centered on consecutive lines.

You can also update the message configuration dynamically:
* `msg:set(emit_table_key, value)`: Set an emit table key
* `value = msg:get(emit_table_key)`: Get an emit table key value
* `msg:set_emit(obj)`: Set (or replace) the emit object
* `msg:set_line_spacing(value)`: Set the line spacing
* `msg:set_zoom(value)`: Set the zoom multiplier

#### Example

```lua
-- Emitting a static centered title
local zoom = 4  -- letter size
local e = Emit:new({tag=1})
e:count(zoom*4)
e:radius(1)
e:ds(0, 0)
e:theta(0, math.pi)
e:life(5000)
e:color(0, 0.8, 0.4, 0.2, .1, 0)
e:blender(Vis.BLEND_EASING)

local m = Message:new{emit=e, line_spacing=1.5, zoom=zoom}

-- Emit the text the instant the script begins
m:emit_center_lines(1, {
    "Une vie à t'aimer",
    "Lorien Testard"
}, W/2, H/2 + 50)


-- Dynamically changing the emit object's properties
local me = e:clone()
local m2 = Message:new{emit=me, zoom=2}

for onms, frame_info in pairs(MIDI.to_timing(intro)) do
    -- Modify the underlying emit instance directly
    me:life(frame_info.duration)

    local msg = ("%d id %d %s"):format(
        onms,
        frame_info.frames[1].id,
        frame_info.frames[1].lyric or "<none>")

    m2:emit_lines(now + onms, { msg }, 0, 0)
end
```

## Planned Features

I plan to add a way to adjust emit frames according to the song. This involves
calculating a fast-Fourier transform (FFT) in addition to querying absolute
volume (or envelope) information.

Note that because the particle engine runs at a specific fame-rate (default 30),
this reduces the resolution made available to the particles.

## Credits

This project makes use of the song "Royalty", published by NCS:

```
Song: Egzod, Maestro Chives, Neoni - Royalty [NCS Release]
Music provided by NoCopyrightSounds
Free Download/Stream: http://ncs.io/Royalty
Watch: http://ncs.lnk.to/RoyaltyAT/youtube
```
