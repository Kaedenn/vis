
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "async.h"
#include "audio.h"
#include "clargs.h"
#include "command.h"
#include "defines.h"
#include "draw.h"
#include "emitter.h"
#include "helper.h"
#include "particle_extra.h"
#include "plist.h"
#include "scheduler.h"
#include "script.h"

#define VIS_CMD_DELAY_NSTEPS 5

plist_t particles = NULL;
sched_ctx ctx = NULL;

void finalize(void);
void mainloop(void);

void animate_particle(plist_node_t node);
void display(void);
void timeout(void);

void tick(UNUSED_PARAM(void* arg)) {
  display();
  timeout();
}

int main(int argc, char* argv[]) {
  SDL_Surface* screen = NULL;
  ctx = malloc(sizeof(struct sched_ctx_));
  srand((unsigned)time(NULL));
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    eprintf("unable to initialize: %s", SDL_GetError());
    exit(1);
  }
  atexit(finalize);
  screen = SDL_SetVideoMode(VIS_WIDTH, VIS_HEIGHT, 32,
                            SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL);
  if (screen == NULL) {
    eprintf("unable to create screen: %s", SDL_GetError());
    exit(1);
  }
  
  particles = plist_new();
  
  emitter_setup(particles);
  audio_init();
  
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClearDepth(1.0f);
  glViewport(0, 0, VIS_WIDTH, VIS_HEIGHT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, VIS_WIDTH, VIS_HEIGHT, 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
  glEnable(GL_TEXTURE_2D);
  glLoadIdentity();
  
  argparse(argc, argv);
  
  if (args.interactive) {
    command_setup(particles);
  }
  
  if (args.scriptfile) {
    emitter_schedule(load_script(args.scriptfile));
  }
  
  scheduler_reset(ctx);
  schedule_every(ctx, VIS_FPS_LIMIT, tick, NULL);
  
  mainloop();
  
  return 0;
}

void mainloop(void) {
  SDL_Event e;
  Uint32 lasttime = SDL_GetTicks();
  while (TRUE) {
    while (SDL_PollEvent(&e)) {
      /* insane optimization trick: if it's time to emit, do it now */
      if (SDL_GetTicks() - lasttime >= 1000/VIS_FPS_LIMIT) goto L_ANIMATE;
      /*scheduler_tick(ctx);*/
      switch (e.type) {
        case SDL_QUIT: {
          return;
        } break;
        default: { } break;
      }
    }
/*    scheduler_tick(ctx);*/
    if (SDL_GetTicks() - lasttime >= 1000/VIS_FPS_LIMIT) {
L_ANIMATE:
      lasttime = SDL_GetTicks();
      display();
      timeout();
    }
  }
}

void animate_particle(plist_node_t node) {
  /* TODO: add passive forces */
  particle_t p = node->particle;
  pextra_t pe = (pextra_t)(p->extra);
  double life, lifetime;
  double alpha = 1;
  life = particle_get_life(p);
  lifetime = particle_get_lifetime(p);
  switch (pe->blender) {
    /* default blend is linear */
    case VIS_BLEND_LINEAR: {
      alpha = linear_blend(life, lifetime);
    } break;
    case VIS_BLEND_QUADRATIC: {
      alpha = quadratic_blend(life, lifetime);
    } break;
    case VIS_BLEND_NEGGAMMA: {
      alpha = neggamma_blend(life, lifetime);
    } break;
    case VIS_NO_BLEND: {
      alpha = no_blend(life, lifetime);
    } break;
    case VIS_NBLENDS:
    default: { } break;
  }
  
  glBegin(GL_POLYGON);
  glColor4d(pe->r, pe->g, pe->b, alpha);
  draw_diamond(p->x, p->y, p->radius);
  glEnd();
  
  particle_tick(p);
  if (!particle_is_alive(p)) {
    plist_remove(particles, node);
  }
}

void display(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  plist_safe_foreach(particles, animate_particle);
  SDL_GL_SwapBuffers();
}

void timeout(void) {
  static int delayctr = 0;
  if (args.interactive) {
    if (++delayctr % VIS_CMD_DELAY_NSTEPS == 0) {
      command();
      delayctr = 0;
    }
  }
  emitter_tick();
}

void finalize(void) {
  SDL_Quit();
  plist_free(particles);
  audio_close();
}

