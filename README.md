
# Hand-Crafted Particle Visualization to Music, via OpenGL and Lua

## Background information

This project was created to investigate the question, "can you simulate
synesthesia for someone who does not have synesthesia?"

Synesthesia, specifically chromesthesia, is the perception of shapes when
listening to music. The exact shapes, their positions, colors, and behaviors
are unique to the individual. Therefore, this project does not attempt to
simulate synesthesia in general, but rather simulate a specific person's
experience.

Chromesthesia has been described as "a fireworks display for the mind" and can
be genuinely enjoyable. Unfortunately, I do not have chromesthesia. Therefore,
I should be the perfect use case for this project.

Unfortunately, as the developer, this means I need to determine what someone
with chromesthesia would experience.

<details>
<summary><strong>Rules of Chromesthesia</strong></summary>

To ensure the experience I create closely matches real chromesthesia, I have
decided to abide by the following two rules:

1. Shape Duration: Shapes should only appear on-screen for the duration of the
   note, instrument, or chord. Once the sound stops or changes, the shape should
   cease or change.
2. Shape Consistency: The same sound on the same instrument should give the same
   shape regardless of when it's played.

</details>

## Compiling this thing

This project depends on OpenGL and Lua. There are several `$(MAKE)` targets,
including `all`, `debug`, `valgrind`, etc. A simple `make all` should do the
trick just fine.

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

There are several ways to run this thing: <br/>
1) With no arguments, using click-and-drag <br/>
2) With the `-l` argument and a path to a `.lua` script <br/>
3) With no arguments, using stdin for commands <br/>

#### Click-and-drag method

First, run the program with no arguments. Then, click and hold with the left
mouse button and move the mouse around the screen. This will cause small circles
of particles to appear. It is possible to change the parameters of this
emission, but that requires a script file (the second method)

#### Script mode

This is the most recommended way to run the program. I recommend running the
following script:

```
$ ./vis -l test/4_random_long.lua
```

This gives you a good taste of what the particle engine can do, although it
leaves out a good number of details.

The full documentation of script mode is below, see *Writing your own scripts*

<details>
<summary><strong>Command Mode</strong></summary>

This mode exists for historical reasons and really isn't intended to be used
outside of debugging. That said, there are a number of commands supported:

`command "help" [argument] `: Invoked as either `help` or
`help <command>`, this command lists all valid commands and a small
description of each. As a special case, `help emit` lists extended
information on the emit function.

`command "emit" <arguments>`: Emits a bundle of particles specified by the
arguments given. Type `help emit` to see the arguments; there are too many
to list here.

`command "kick" <n>`: Generates a preconfigured emit of `<n>` particles.

`command "snare" <n>`: Generates a preconfigured emit of `<n>` particles.

`command "strum" <n>`: Generates a preconfigured emit of `<n>` particles.

`command "rain" <n>`: Generates a preconfigured emit of `<n>` particles.

`command "lua" <file>`: Loads and executes the lua script `<file>`.

`command "audio" <file>`: Loads and plays the sound file `<file>`,
which unfortunately must be a path to a WAV file. No other formats are
supported yet.

`command "exit"`: Terminates the application. Pressing either `^D` or
`ESC` also terminates the application.

</details>

### Writing your own scripts

This is the fun part. There are three modules available: Vis, VisUtil, and Emit.

### A note about uncertainty:

The vast majority of values have an uncertainty counterpart, which is used to
adjust the particle emission. For example, the uncertainty in horizontal
position `x` is denoted as `ux`, and every particle emitted by this
table will be positioned at a random spot between `x - ux` and
`x + ux`.  Position, radius, angle, life, and color all have uncertainty
values which all follow this rule.

<details>
<summary><strong>The <code>Vis</code> Module</strong></summary>

#### `module Vis = require("Vis")`

This is the core library. Its fields are:

`userdata Vis.flist`: The `frame list` structure. In order to schedule
emits, the `vis` executable needs to know where the scheduling is stored.
This is that location.

`userdata Vis.script`: This field is present in order to perform advanced
operations, such as scheduling callbacks (strings of Lua script to be executed
after a certain amount of time has passed). This field is `nil` when the
advanced features are not available, which is only when loading scripts via
the `load <scriptfile>` command in interactive mode.

`function Vis.debug(...)`: For debugging purposes, this function prints
out its arguments to the debugging stream if and only if the program was
compiled with debugging enabled (via `make debug`).

`function Vis.command(Vis.flist, when, command)`: Executes the
interactive-mode command `command` after `when` milliseconds has
passed.

`function Vis.emit(...)`: Do not use; see `VisUtil.make_emit_table`,
`VisUtil.emit_table`, the `Emit` class, as well as the documentation
on emit tables.

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
specified number of frames, then unmute and restart the audio track at the
beginning (0ms). This is needed when the audio track does not start playing
immediately and ensures both the audio and visual effects have enough time to
synchronize.

`function Vis.delay(Vis.flist, when, frame_count)`: Delays the visual effects
for the specified number of frames, while allowing the audio to continue. Use
this if the audio takes longer to load or has "dead time" before the song
begins.

`function Vis.bgcolor(Vis.script, r, g, b)`: Sets the background color to
the values given. Each value is between 0 and 1.

`function Vis.mutate(Vis.flist, when, func, coefficient)`: Schedules the
mutation given by `func` (see constants `Vis.MUTATE_*`) with
coefficient `coefficient` after `when` milliseconds have passed.
Mutates affect all living particles on the screen.

`function Vis.callback(Vis.flist, when, Vis.script, lua_script)`: Executes
`lua_script` in the same context as the calling file after `when`
milliseconds have passed.

`function Vis.fps()`: Returns the current derived frames per second, which
should be fairly close to `Vis.FPS_LIMIT`.

`function Vis.settrace(Vis.script, emit_table)`: Change what happens when the
user clicks and drags on the screen. The `emit_table` must be a native Lua
`table` and not an `Emit` instance.

`function Vis.frames2msec(frame_number)`: Converts argument `frame_number` to
milliseconds.

`function Vis.msec2frames(milliseconds)`: Converts argument `milliseconds` to
a frame number.

`constant Vis.FPS_LIMIT`: The intended frames-per-second at which this
program runs. At the time of writing, this is set to 30.

`constant Vis.WIDTH`: The width of the program window. Currently set to 800
but can be changed on the command-line.

`constant Vis.HEIGHT`: The height of the program window. Currently set to
600 but can be changed on the command-line.

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

`constant Vis.MUTATE_AGE`: Sets the age of the particles to the product of
their total lifetime and the coefficient given.

`constant Vis.MUTATE_OPACITY`: Sets the particle base opacity to the
coefficient given. The blending functions are multiplied by this value.

`constant Vis.MUTATE_TAG_SET`: Tags all particles by setting their tag
value to the integer given.

`constant Vis.MUTATE_TAG_INC`: Increases all particles' tags by one.

`constant Vis.MUTATE_TAG_DEC`: Decreases all particles' tags by one.

`constant Vis.MUTATE_TAG_ADD`: Adds the value given to all particles'
tags.

`constant Vis.MUTATE_TAG_SUB`: Subtracts the value given from all
particles' tags.

`constant Vis.MUTATE_TAG_MUL`: Multiplies all particles' tags by the value
given.

`constant Vis.MUTATE_TAG_DIV`: Divides all particles' tags by the value
given.

`constant Vis.MUTATE_PUSH_IF`: Apply `Vis.MUTATE_PUSH` if the
particles' tag and the tag value specified satisfies the condition specified
(see `Vis.MUTATE_IF_*` below)

`constant Vis.MUTATE_PUSH_DX_IF`: Apply `Vis.MUTATE_PUSH_DX` if the
condition specified evaluates to true against the particle and the tag given.

`constant Vis.MUTATE_PUSH_DY_IF`: As above, with `Vis.MUTATE_PUSH_DY`

`constant Vis.MUTATE_SLOW_IF`: As above, with `Vis.MUTATE_SLOW`

`constant Vis.MUTATE_SHRINK_IF`: As above, with `Vis.MUTATE_SHRINK`

`constant Vis.MUTATE_GROW_IF`: As above, with `Vis.MUTATE_GROW`

`constant Vis.MUTATE_AGE_IF`: As above, with `Vis.MUTATE_AGE`

`constant Vis.MUTATE_OPACITY_IF`: As above, with `Vis.MUTATE_OPACITY`

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

`constant Vis.FORCE_FRICTION_COEFF`: The strength of friction, from 0 to 1.

`constant Vis.FORCE_GRAVITY_FACTOR`: The strength of gravity, from 0 to 1.

`constant Vis.NFRAMES`: The maximum number of frames that can be
scheduled. This is set to `1048576` which equates to a little over nine
hours and forty-two minutes.

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

</details>

<details>
<summary><strong>The <code>VisUtil</code> Module</strong></summary>

#### `module VisUtil = require("visutil")`

This module is pure Lua and resides in `lua/visutil.lua`. Have a look there
to see how everything is implemented.  The emit table granted by the
`VisUtil` functions has a large number of members, all of which are public
and freely modifiable.

`function VisUtil.make_emit_table()`: Returns a newly-constructed emit
table with the position set to the center of the screen, radius set to 1,
color set to white, and blend set to linear.

`function VisUtil.center_emit_table(table, x, y, ux, uy)`: Centers the emit
table `table` at `x` and `y`, with a variance of `ux` and
`uy`. With only one argument, the table is centered at the center of the
screen and the variances are set to zero. Negative values wrap around the
screen from the opposite side; `-VIS_WIDTH/4 == 3*VIS_WIDTH/4`

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

</details>

<details>
<summary><strong>The Emit Table and <code>Emit</code> Module</strong></summary>

#### The emit table

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

`floats ds, uds`: The initial speed of the particles to emit, measured in
pixels per frame.

`floats theta, utheta`: The angle at which to emit the particles.

`integers life, ulife`: The lifetime, in milliseconds, of the particles
to emit.

`floats r, g, b, ur, ug, ub`: The color of the particles to
emit. It is recommended to use `VisUtil.color_emit_table` or
`Emit:color` over using these values directly.

`enumeration force`: One of the `Vis.FORCE_` values. Passing a value
other than these will cause errors and, in certain circumstances, may cause
the program to terminate.

`enumeration limit`: One of the `Vis.LIMIT_` values. Passing a value
other than these will cause errors and, in certain circumstances, may cause
the program to terminate.

`enumeration blender`: One of the `Vis.BLEND_` values. As above,
passing values other than these will cause errors and, in certain
circumstances, may cause the program to terminate.

#### `module Emit = require('emit')` and the `Emit` class

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

`e:ds(ds, uds)`: Configure the emit's initial velocity. The second
parameter defaults to zero.

`e:theta(theta, utheta)`: Configure the emit's angle. The second parameter
defaults to zero.

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

</details>

<details>
<summary><strong>Mutation</strong></summary>

Mutates are a very powerful type of frame and provide a way to modify either
all particles on screen or only a subset, using tag modification and
conditional mutates. Mutates only affect particles already on screen; they do
not affect particles yet to be emitted. There are three kinds of mutates,
each with their own parameter list.

#### Standard mutate

`Vis.mutate(Vis.flist, <when>, <operation>, <factor-or-coefficient>)`

This applies `<operation>`, one of the `Vis.MUTATE_` values, to all
living particles currently on screen. Nearly all the operations require a
double-precision number, `<factor-or-coefficient>`. For example, the
operation `Vis.MUTATE_GROW` multiples all particles' radii by the
coefficient given. Note that this excludes the `Vis.MUTATE_TAG_`
operations and the `Vis.MUTATE_IF_` values as those are covered below.

#### Tag modification

`Vis.mutate(Vis.flist, <when>, <operation>, <value>)` where
`<operation>` is one of the `Vis.MUTATE_TAG_` values.

This modifies all active particles currently on screen, adjusting their tag
by the operation and value given. For example, `Vis.MUTATE_TAG_SET` with a
value of `5` will set all active particles' tags to the number 5.

#### Conditional mutate

`Vis.mutate(Vis.flist, <when>, <operation>_IF, <factor-or-coefficient>, <condition>, <tag>)`

This applies `<operation>` to all particles on screen if (and only if)
both the particle's tag and the `<tag>` value given satisfy the
`<condition>` given. For example,
`Vis.mutate(Vis.flist, 50, Vis.MUTATE_PUSH_IF, 2, Vis.MUTATE_IF_EQ, 1)`
will double all particles' velocities if and only if the particle's tag is
equal to 1.

</details>

<details>
<summary>Planned Features</summary>

I plan to add a way to adjust emit frames according to the song. This involves
calculating a fast-Fourier transform (FFT) in addition to querying absolute
volume (or envelope) information.

Note that because the particle engine runs at a specific fame-rate (default 30),
this reduces the resolution made available to the particles.

</details>

## Credits

This project makes use of the song "Royalty", published by NCS:

```
Song: Egzod, Maestro Chives, Neoni - Royalty [NCS Release]
Music provided by NoCopyrightSounds
Free Download/Stream: http://ncs.io/Royalty
Watch: http://ncs.lnk.to/RoyaltyAT/youtube
```
