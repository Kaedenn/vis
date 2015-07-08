
# Hand-Crafted Particle Visualization to Music, via SDL, OpenGL, and Lua

### There's a demo! You want a demo? Sure you want a demo!

Take a look in the output directory for a file named "bowser.avi". This file
will, over time, become the final product for this project.

## A little bit of background information

This project is a glorified script-able particle engine intended for providing
a complex visualization for songs. Because "trans-coding" audio to a MIDI
format is practically impossible, all of the scheduling--that is, writing the
script to emit the particles--must be done by hand. This is where a keen ear
and substantial knowledge of music theory comes in, neither of which I claim to
have.

This leads to an interesting dichotomy: one one hand, simpler songs would be
easier to schedule, but they would lead to a fairly boring visualization. On
the other hand, however, complex songs require a lot of work to figure out
what's going on. I hope I've hit an acceptable compromise in the song I've
chosen: the Bowser theme from Super Mario World 2: Yoshi's Island. Being a
theme from an old video game, the theme is not overly complex. It also has a
repeat point, meaning I can drag on the visualization for as long as I please.
In this specific project, I schedule two minutes and 43.69 seconds.

## Compiling this thing

This project depends on SDL, OpenGL, and Lua. There are several $(MAKE)
targets, including ```all```, ```debug```, ```valgrind```, etc. A simple
```make all``` should do the trick just fine.

## Running this thing

This program has just one argument for intended usage: ```-l <FILE>```. This
argument loads the Lua script ```<FILE>``` and executes it. For information on
writing scripts, see any of the Lua files in the ```test``` directory or the
section ```Writing your own scripts``` below.

### Running the final product

To see the final product, run this program as the following:
```$ ./vis -l lua/bowser.lua```

### Other invocations

There are several ways to run this thing: <br/>
1) With no arguments, using click-and-drag <br/>
2) With the ```-l``` argument and a path to a ```.lua``` script <br/>
3) With no arguments, using stdin for commands <br/>

#### Click-and-drag method

First, run the program with no arguments. Then, click and hold with the left
mouse button and move the mouse around the screen. This will cause small
circles of particles to appear. It is possible to change the parameters of
this emission, but that requires a script file (the second method)

#### Script mode

This is the most recommended way to run the program. I recommend running the
following script:
```$ ./vis -l test/4_random_long.lua```
This gives you a good taste of what the particle engine can do, although it
leaves out a good number of details.

The full documentation of script mode is below, see *Writing your own scripts*

#### Command mode

This mode exists for historical reasons and really isn't intended to be used
outside of debugging.

### Writing your own scripts

This is the fun part. There are two distinct APIs available: Vis and VisUtil.

```Vis = require("Vis")```

This is the core library. Its fields are:

```userdata Vis.flist```: The ```frame list``` structure. In order to schedule
emits, the ```vis``` executable needs to know where the scheduling is stored.
This is that location.

```userdata Vis.script```: This field is present in order to perform advanced
operations, such as scheduling callbacks (strings of Lua script to be executed
after a certain amount of time has passed). This field is ```nil``` when the
advanced features are not available, which is only when loading scripts via
the ```load <scriptfile>``` command in interactive mode.

```function Vis.debug(...)```: For debugging purposes, this function prints out its
arguments to the debugging stream if and only if the program was compiled with
debugging enabled (via ```make debug```).

```function Vis.command(Vis.flist, when, command)```: Executes the
interactive-mode command ```command``` after ```when``` milliseconds has passed.

```function Vis.emit(...)```: Do not use; see ```VisUtil.make_emit_table``` and
```VisUtil.emit_table```, as well as the documentation on emit tables.

```function Vis.audio(path)```: Load the audio file given by ```path```.

```function Vis.play()```: Plays the current audio file.

```function Vis.pause()```: Pauses the current audio file.

```function Vis.seek(hundreths-of-a-second)```: Seeks the current audio file to the
offset given by ```hundredths-of-a-second```.

```function Vis.seekms(Vis.flist, when, milliseconds)```: Sets the current
position of the schedule to ```milliseconds``` when ```when``` milliseconds have
passed.

```function Vis.seekframe(Vis.flist, when, frame_number)```: Sets the current
position of the schedule to ```frame_number``` after ```when``` milliseconds
have passed. See ```Vis.frames2msec``` and ```Vis.msec2frames``` to convert
between frames and milliseconds.

```function Vis.bgcolor(Vis.script, r, g, b)```: Sets the background color to the
values given. Each value is between 0 and 1.

```function Vis.mutate(Vis.flist, when, func, coefficient)```: Schedules the
mutation given by ```func``` (see constants ```Vis.MUTATE_*```) with coefficient
```coefficient``` after ```when``` milliseconds have passed. Mutates affect all
living particles on the screen.

```function Vis.callback(Vis.flist, when, Vis.script, lua_script)```: Executes
```lua_script``` in the same context as the calling file after ```when```
milliseconds have passed.

```function Vis.fps()```: Returns the current derived frames per second, which
should be fairly close to ```Vis.FPS_LIMIT```.

```function Vis.settrace(...)```: Not yet documented.

```function Vis.frames2msec(frame_number)```: Converts argument
```frame_number``` to milliseconds.

```function Vis.msec2frames(milliseconds)```: Converts argument
```milliseconds``` to a frame number.

```constant Vis.FPS_LIMIT```: The intended frames-per-second at which this
program runs. At the time of writing, this is set to 30.

```constant Vis.WIDTH```: The width of the program window. Currently set to 800
but can be changed.

```constant Vis.HEIGHT```: The height of the program window. Currently set to
600 but can be changed.

```constant Vis.DEFAULT_BLEND```: Equal to Vis.BLEND_NONE

```constant Vis.BLEND_NONE```: Particles do not fade out; they simply vanish
suddenly once their lifetime runs out.

```constant Vis.BLEND_LINEAR```: Particles fade out at a constant rate during
their lifetime.

```constant Vis.BLEND_QUADRATIC```: Particles start fading slowly and fade
faster as they age.

```constant Vis.BLEND_NEGGAMMA```: Particles fade almost suddenly at the end of
their lifetime.

```constant Vis.NBLENDS```: Equal to the number of possible blend effects.

```constant Vis.DEFAULT_FORCE```: Particles do not undergo any additional force
as they age.

```constant Vis.FORCE_FRICTION```: Particles gradually slow down as they age.

```constant Vis.FORCE_GRAVITY```: Particles gradually accelerate towards the
bottom of the screen as they age.

```constant Vis.NFORCES```: Equal to the number of possible force effects.

```constant Vis.DEFAULT_LIMIT```: Particles are free to move off of the screen.

```constant Vis.LIMIT_BOX```: Particles stop at the edges of the screen.

```constant Vis.LIMIT_SPRINGBOX```: Particles bounce off the edges of the
screen.

```constant Vis.NLIMITS```: Equal to the number of possible limit choices.

```constant Vis.MUTATE_PUSH```: Accelerates the particles by the coefficient
given.

```constant Vis.MUTATE_SLOW```: Decelerates the particles by the coefficient
given.

```constant Vis.MUTATE_SHRINK```: Shrinks the particles by the coefficient
given.

```constant Vis.MUTATE_GROW```: Grows the particles by the coefficient given.

```constant Vis.MUTATE_PUSH_DX```: Accelerates the particles in the horizontal
direction by the coefficient given.

```constant Vis.MUTATE_PUSH_DY```: Accelerates the particles in the vertical
direction by the coefficient given.

```constant Vis.MUTATE_AGE```: Sets the age of the particles to the product of
their total lifetime and the coefficient given.

```constant Vis.MUTATE_OPACITY```: Sets the particle base opacity to the
coefficient given. The blending functions are multiplied by this value.

```constant Vis.NMUTATES```: Equal to the number of possible mutate choices.

```constant Vis.FORCE_FRICTION_COEFF```: The strength of friction, from 0 to 1.

```constant Vis.FORCE_GRAVITY_FACTOR```: The strength of gravity, from 0 to 1.

```module VisUtil = require("visutil")```

This module is pure-Lua and resides in ```lua/visutil.lua```. Have a look there
to see how everything is implemented.

```function VisUtil.make_emit_table()```: Returns a newly-constructed emit table
with the position set to the center of the screen, radius set to 1, color set to
white, and blend set to linear.

```function VisUtil.center_emit_table(table, x, y, ux, uy)```: Centers the emit
table ```table``` at ```x``` and ```y```, with a variance of ```ux``` and
```uy```. With only one argument, the table is centered at the center of the
screen and the variances are set to zero.

```function VisUtil.color_emit_table(table, r, g, b, ur, ug, ub)```: Sets the
color to the values given, with variances given by ```ur```, ```ug```, and
```ub```. Colors can be either from 0 to 1, or from 0 to 255. Omitted values
are set to zero.

```function VisUtil.emit_table(table)```: Invokes ```Vis.emit```, passing the
table given by ```table```.

```function VisUtil.seek_to(milliseconds)```: Invokes both ```Vis.seek()``` and
```Vis.seekms``` with the value given.

```function VisUtil.set_trace_table(table)```: Invokes ```Vis.settrace```,
passing the table given. This is how you modify the click-and-drag emission
parameters.

#### The emit table

The emit table granted by the ```VisUtil``` functions has a large number of
members, all of which are public and freely modifiable.

Uncertainty:

The vast majority of values have an uncertainty counterpart, which is used to
adjust the particle emission. For example, the uncertainty in horizontal
position ```x``` is denoted as ```ux```, and every particle emitted by this
table will be positioned at a random spot between ```x - ux``` and ```x + ux```.
Position, radius, angle, life, and color all have uncertainty values which all
follow this rule.

```integer T.count```: The number of particles to emit.

```integer T.when```: The time (in milliseconds) at which to emit this frame.

```integers T.x, T.ux```: The horizontal position of emission.

```integers T.y, T.uy```: The vertical position of emission.

```floats T.radius, T.uradius```: The radius, in pixels, of the particles to
emit.

```floats T.ds, T.uds```: The initial speed of the particles to emit.

```floats T.theta, T.utheta```: The angle at which to emit the particles.

```integers T.life, T.ulife```: The lifetime, in milliseconds, of the particles
to emit.

```floats T.r, T.g, T.b, T.ur, T.ug, T.ub```: The color of the particles to
emit. It is recommended to use ```VisUtil.color_emit_table``` over using these
values directly.

```enumeration T.force```: One of the ```Vis.FORCE_*``` values.

```enumeration T.limit```: One of the ```Vis.LIMIT_*``` values.

```enumeration T.blender```: One of the ```Vis.BLEND_*``` values.

## Credit where credit is due

Thank you SNESmusic.org for the Bowser theme, retrieved at
http://snesmusic.org/v2/profile.php?profile=set&selected=2757
on the 8th of October 2011.

Thank you Kainui from ##math at irc.freenode.net for helping me with the
blending functions. It would seem my algebra skills are lacking since I left
university.
