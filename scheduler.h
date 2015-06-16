
#ifndef VIS_SCHEDULER_HEADER_INCLUDED_
#define VIS_SCHEDULER_HEADER_INCLUDED_

#include <SDL/SDL.h>

typedef unsigned long msec_t;
typedef void(*sched_cb)(void*);

typedef struct sched_ctx_ {
    msec_t base;
    unsigned long tick;
    msec_t when;
    sched_cb func;
    void* arg;
} *sched_ctx;

void scheduler_reset(sched_ctx ctx);

void schedule_every(sched_ctx ctx, msec_t msec, sched_cb cb, void* arg);

void scheduler_tick(sched_ctx ctx);

#endif

