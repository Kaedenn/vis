
#ifndef VIS_DEFINES_HEADER_INCLUDED_
#define VIS_DEFINES_HEADER_INCLUDED_ 1

#define _POSIX_C_SOURCE 200809L

#ifdef __GNUC__
/* GCC-specific extensions to make things a little easier */
#define UNUSED_PARAM(param) param __attribute__((unused))
#define UNUSED_VARIABLE(var) (void)var
#define NORETURN __attribute__((noreturn))
#else
#define UNUSED_PARAM(param) param
#define UNUSED_VARIABLE(var) (void)var
#define NORETURN
#endif

/* debugging levels */
#define DEBUG_NONE 0
#define DEBUG_VERBOSE 1
#define DEBUG_DEBUG 2
#define DEBUG_INFO 3
#define DEBUG_TRACE 4

#ifndef DEBUG
#define DEBUG DEBUG_DEBUG
#endif

#if DEBUG >= DEBUG_DEBUG
#define DEBUG_EXPRESSION(x) (x)
#else
#define DEBUG_EXPRESSION(x)
#endif

/* file ran after Vis module construction and before all other code */
#ifndef LUA_STARTUP_FILE
#define LUA_STARTUP_FILE "lua/startup.lua"
#endif

/* physics globals */
#define VIS_FORCE_FRICTION_COEFF 0.99
#define VIS_FORCE_GRAVITY_FACTOR 0.03

/* constants for the frame array's length */
/* use -DVIS_FRAMES=N to override */
#ifndef VIS_NFRAMES
#define VIS_FRAMES_NMINS 15
#define VIS_FRAMES_NSECS 0
#define VIS_NFRAMES ((VIS_FRAMES_NMINS * 60 + VIS_FRAMES_NSECS) * VIS_FPS_LIMIT)
#endif

/* window size */
#define VIS_WIDTH 800
#define VIS_HEIGHT 600

/* interactive mode: poll async once per 5 frames */
#define VIS_CMD_DELAY_NSTEPS 5

/* size of fuzzy edges around particle p */
#define BOX_FUDGE(p) (0)

/* physics */
#define VIS_FORCE_FRICTION_COEFF 0.99
#define VIS_FORCE_GRAVITY_FACTOR 0.03

/* default frame rate */
#define VIS_FPS_LIMIT 60

/* audio constants */
#define VIS_AUDIO_FREQ 44100
#define VIS_AUDIO_SAMPLES 512
#define VIS_AUDIO_CHANNELS 2

/* used for commands */
#define VIS_BUFFER_LEN 4096

/* initial capacity for particles */
#define VIS_PLIST_INITIAL_SIZE (2 * 1024 * 1024)

/* convenience */
#ifndef BOOL
#define BOOL int
#endif

#if !defined(TRUE) && !defined(FALSE)
#define TRUE 1
#define FALSE 0
#endif

#ifndef M_PI
#define M_PI 3.141592653579
#endif

#endif
