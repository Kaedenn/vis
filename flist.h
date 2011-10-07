
#ifndef VIS_FLIST_HEADER_INCLUDED_
#define VIS_FLIST_HEADER_INCLUDED_ 1

#include "frame.h"

/* Frame List: an array of linked lists!
  This structure has an array of VIS_NFRAMES pointers to flist_nodes.
  
  In the current setup, that's ~35kB for one array.
*/

/* constants for frame types */
#define VIS_FTYPE_EMIT 0
#define VIS_FTYPE_CMD 1

/* constants for the frame array */
/* use -DVIS_FRAMES=N to override */
#ifndef VIS_NFRAMES
# define VIS_FRAMES_NMINS 10
# define VIS_FRAMES_NSECS 0
# define VIS_NFRAMES ((VIS_FRAMES_NMINS*60+VIS_FRAMES_NSECS)*VIS_FPS_LIMIT)
#endif

/* constants for Guns N' Roses: Sweet Child O' Mine */
#define VIS_SCOM_NMINS 5
#define VIS_SCOM_NSECS 55
#define VIS_SCOM_TIME (VIS_SCOM_NMINS*60+VIS_SCOM_NSECS)
#define VIS_SCOM_NFRAMES (VIS_SCOM_TIME*VIS_FPS_LIMIT)

typedef unsigned int fnum_t;
typedef unsigned int frame_type_t;

typedef struct flist_node {
  frame_type_t type;
  union {
    frame_t frame;
    const char* cmd;
  } data;
  struct flist_node* next;
} *flist_node_t;

typedef struct flist {
  flist_node_t frames[VIS_NFRAMES]; /* with apologies to the hardware */
  fnum_t curr_frame;
} *flist_t;

flist_t flist_new(void);
void flist_free(flist_t fl);
void flist_insert_emit(flist_t fl, fnum_t where, frame_t what);
void flist_insert_cmd(flist_t fl, fnum_t where, const char* what);
void flist_clear(flist_t fl);
void flist_restart(flist_t fl);

flist_node_t flist_tick(flist_t fl);

#endif

