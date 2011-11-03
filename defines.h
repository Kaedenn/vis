
#ifndef VIS_DEFINES_HEADER_INCLUDED_
#define VIS_DEFINES_HEADER_INCLUDED_ 1

#define UNUSED_PARAM(param) param __attribute__((unused))

/* force functions */
typedef enum {
  /* default force is no force */
  VIS_DEFAULT_FORCE = 0,
  VIS_FRICTION,
  VIS_GRAVITY,
  VIS_NFORCES
} force_t;

/* limiting functions */
typedef enum {
  /* default limit is no limit */
  VIS_DEFAULT_LIMIT = 0,
  VIS_BOX,
  VIS_SPRINGBOX,
  VIS_NLIMITS
} limit_t;

/* mutation functions */
typedef enum {
  VIS_DEFAULT_MUTATE = 0,
  VIS_SHRINK_LINEAR
} mutate_t;

/* alpha-channel blending functions */
typedef enum {
  VIS_NO_BLEND = 0,
  /* default blend is linear */
  VIS_DEFAULT_BLEND = 1,
  VIS_BLEND_LINEAR = 1,
  VIS_BLEND_QUADRATIC,
  VIS_BLEND_NEGGAMMA,
  VIS_NBLENDS
} blend_t;

/* physics */
#define VIS_FRICTION_COEFF 0.99
#define VIS_GRAVITY_FACTOR 0.03

/* window size */
#define VIS_WIDTH 800
#define VIS_HEIGHT 600

/* frame rate */
#define VIS_FPS_LIMIT 30
#define VIS_FPDS_LIMIT (VIS_FPS_LIMIT / 10)
#define VIS_FPCS_LIMIT (VIS_FPS_LIMIT / 100.0)

/* timing constants */
#define VIS_10MS (10)
#define VIS_MSEC_PER_FRAME (1000 / VIS_FPS_LIMIT)

/* error in frame rate: delay 1ms every 3 frames */
#define VIS_FPMS_ERROR_MS 1
#define VIS_FPMS_ERROR_FRAMES 3

/* audio constants */
#define VIS_AUDIO_FREQ 22050
#define VIS_AUDIO_SAMPLES 512
#define VIS_AUDIO_CHANNELS 2

/* used for commands */
#define VIS_BUFFER_LEN 4096

/* convenience */
#ifndef BOOL
#define BOOL int
#endif

#if !defined(TRUE) || !defined(FALSE)
#define TRUE 1
#define FALSE 0
#endif

#endif

