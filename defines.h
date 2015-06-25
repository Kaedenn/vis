
#ifndef VIS_DEFINES_HEADER_INCLUDED_
#define VIS_DEFINES_HEADER_INCLUDED_ 1

#ifdef __GNUC__
/* GCC-specific extensions to make things a little easier */
#define UNUSED_PARAM(param) param __attribute__((unused))
#define REQUEST_INLINE __attribute__((always_inline))
#define UNUSED_VARIABLE(var) (void)var
#else
#define UNUSED_PARAM(param) param
#define REQUEST_INLINE
#define UNUSED_VARIABLE(var) (void)var
#endif

#include "types.h"

/* allow toggling of the always_inline directive */
#ifdef VIS_SKIP_MANUAL_OPTIMIZATION
#undef REQUEST_INLINE
#define REQUEST_INLINE
#endif

/* physics globals */
#define VIS_FORCE_FRICTION_COEFF 0.99
#define VIS_FORCE_GRAVITY_FACTOR 0.03

/* constants for the frame array's length */
/* use -DVIS_FRAMES=N to override */
#ifndef VIS_NFRAMES
# define VIS_FRAMES_NMINS 10
# define VIS_FRAMES_NSECS 0
# define VIS_NFRAMES ((VIS_FRAMES_NMINS*60+VIS_FRAMES_NSECS)*VIS_FPS_LIMIT)
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

/* frame rate */
#define VIS_FPS_LIMIT 30
#define VIS_FPDS_LIMIT (VIS_FPS_LIMIT / 10.0)
#define VIS_FPCS_LIMIT (VIS_FPS_LIMIT / 100.0)
#define VIS_FPMS_LIMIT (VIS_FPS_LIMIT / 1000.0)

/* timing constants */
#define VIS_10MS (10)
#define VIS_MSEC_PER_FRAME (1000.0 / VIS_FPS_LIMIT)

/* error in frame rate: delay 1ms every 3 frames (FIXME: not 30fps) */
#define VIS_FPMS_ERROR_MS 1
#define VIS_FPMS_ERROR_FRAMES 3

/* audio constants */
#define VIS_AUDIO_FREQ 22050
#define VIS_AUDIO_SAMPLES 512
#define VIS_AUDIO_CHANNELS 2

/* used for commands */
#define VIS_BUFFER_LEN 4096

/* initial capacity for particles */
#define VIS_PLIST_INITIAL_SIZE (1024*1024)

/* (drawing) number of vertices needed per particle */
#define VIS_VTX_PER_PARTICLE 2

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

