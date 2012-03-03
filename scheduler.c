
#include "scheduler.h"

/*
typedef struct sched_ctx_ {
  msec_t base;
  long tick;
  msec_t when;
  sched_cb func;
  void* arg;
} *sched_ctx;
*/

void scheduler_reset(sched_ctx ctx) {
  ctx->base = SDL_GetTicks();
  ctx->tick = 0;
}

void schedule_every(sched_ctx ctx, msec_t msec, sched_cb cb, void* arg) {
  ctx->when = msec;
  ctx->func = cb;
  ctx->arg = arg;
}

/* FIXME: ctx->base > now, if now overflows */
void scheduler_tick(sched_ctx ctx) {
  msec_t now = SDL_GetTicks();
  if ((now - ctx->base) / ctx->when > ctx->tick) {
    ++ctx->tick;
    (ctx->func)(ctx->arg);
  }
}

