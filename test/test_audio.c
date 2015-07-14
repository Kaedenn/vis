
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define min(x,y) (((x) < (y)) ? (x) : (y))
static void mix(void* userdata, Uint8* sream, int length);
static void mainloop(void);

struct global {
    SDL_Window* w;
    SDL_Renderer* r;
    SDL_AudioSpec fmt;
    int ctr;
    int exitnow;
    struct audio_sample {
        Uint8* data;
        Uint32 dpos;
        Uint32 dlen;
    } sample;
};

static struct global g = {0};

int main(int argc, char* argv[]) {
    const char* sndfile = "output/drum4-simple.wav";
    if (argc > 1) {
        sndfile = argv[1];
    }

    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS);
    g.w = SDL_CreateWindow("audio", 0, 0, 120, 120, 0);
    g.r = SDL_CreateRenderer(g.w, -1, 0);

    SDL_zero(g.fmt);

    g.fmt.freq = 44100/2;
    g.fmt.format = AUDIO_S16;
    g.fmt.channels = 2;
    g.fmt.samples = 512;
    g.fmt.callback = mix;
    g.fmt.userdata = NULL;

    if (SDL_OpenAudio(&g.fmt, NULL) < 0) {
        fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
        return 1;
    }

    SDL_AudioSpec wave;
    Uint8* data = 0;
    Uint32 dlen = 0;
    SDL_AudioCVT cvt;
    SDL_zero(wave);
    SDL_zero(cvt);

    if (SDL_LoadWAV(sndfile, &wave, &data, &dlen) == NULL) {
        fprintf(stderr, "Couldn't load %s: %s\n", sndfile, SDL_GetError());
        return 1;
    }

    SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq,
                      AUDIO_S16, 2, 44100/2);
    cvt.buf = malloc(dlen * (Uint32)cvt.len_mult);
    memcpy(cvt.buf, data, dlen);
    cvt.len = dlen;
    SDL_ConvertAudio(&cvt);
#if 1
    SDL_FreeWAV(data);
#else
    cvt.buf = data;
#endif

    SDL_LockAudio();
    g.sample.data = cvt.buf;
    g.sample.dlen = cvt.len_cvt;
    g.sample.dpos = 0;
    SDL_UnlockAudio();

    SDL_PauseAudio(0);

    mainloop();
    fprintf(stderr, "Ticks: %d\n", SDL_GetTicks());
    fprintf(stderr, "Loops: %d\n", g.ctr);

    SDL_AudioQuit();
    SDL_Quit();
}

void mainloop(void) {
    SDL_Event e;
    SDL_zero(e);
    while (!g.exitnow) {
        g.ctr += 1;
        SDL_SetRenderDrawColor(g.r, 0, 0, 0, 0xFF);
        SDL_RenderClear(g.r);
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_ESCAPE) g.exitnow = 1;
                    break;
                case SDL_QUIT: g.exitnow = 1;
                default: break;
            }
        }
        SDL_RenderPresent(g.r);
        SDL_Delay(50);
    }
}

void mix(void* unused, Uint8* stream, int length) {
    SDL_memset(stream, 0, (size_t)length);
    Uint32 amount = min(g.sample.dlen - g.sample.dpos, length);
    SDL_MixAudio(stream, &g.sample.data[g.sample.dpos], amount,
                 SDL_MIX_MAXVOLUME);
    g.sample.dpos += amount;
    if (amount == 0) {
        g.exitnow = 1;
    }
}

