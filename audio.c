
#include "audio.h"
#include "helper.h"

#include <assert.h>
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>

static void mix(void*, Uint8* stream, int length);

typedef struct sample {
  Uint8* data;
  Uint32 dpos;
  Uint32 dlen;
} *sample_t;

typedef struct audio {
  char* file;
  sample_t sample;
} *audio_t;

static audio_t audio = NULL;

void audio_init(void) {
  SDL_AudioSpec fmt;
  ZEROINIT(&fmt);
  
  /* 16-bit stereo at 22kHz */
  fmt.freq = VIS_AUDIO_FREQ;
  fmt.format = AUDIO_S16;
  fmt.channels = VIS_AUDIO_CHANNELS;
  fmt.samples = VIS_AUDIO_SAMPLES;
  fmt.callback = mix;
  fmt.userdata = NULL;
  
  if (SDL_OpenAudio(&fmt, NULL) < 0) {
    eprintf("Unable to open audio: %s\n", SDL_GetError());
    exit(1);
  }
  
  audio = chmalloc(sizeof(struct audio));
  audio->file = NULL;
  audio->sample = chmalloc(sizeof(struct sample));
  audio->sample->data = NULL;
  audio->sample->dpos = 0;
  audio->sample->dlen = 0;
  atexit(SDL_CloseAudio);
}

void audio_open(const char* file) {
  SDL_AudioSpec wave;
  Uint8 *data;
  Uint32 dlen;
  SDL_AudioCVT cvt;
  
  assert(audio != NULL && "must call audio_init first!");
  audio->file = dupstr(file);
  
  /* Load the sound file and convert it to 16-bit stereo at 22kHz */
  if (SDL_LoadWAV(file, &wave, &data, &dlen) == NULL) {
    eprintf("Couldn't load %s: %s\n", file, SDL_GetError());
    return;
  }
  
  DBPRINTF("WAV '%s' loaded: length %d", file, dlen);
  
  SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq,
                    AUDIO_S16, VIS_AUDIO_CHANNELS, VIS_AUDIO_FREQ);
  cvt.buf = malloc(dlen * (Uint32)cvt.len_mult);
  memcpy(cvt.buf, data, dlen);
  cvt.len = (int)dlen;
  SDL_ConvertAudio(&cvt);
  SDL_FreeWAV(data);
  
  if (audio->sample->data) {
    free(audio->sample->data);
  }
  SDL_LockAudio();
  audio->sample->data = cvt.buf;
  audio->sample->dlen = (Uint32)cvt.len_cvt;
  audio->sample->dpos = 0;
  SDL_UnlockAudio();
}

void audio_close(void) {
  free(audio->file);
  free(audio);
  audio = NULL;
}

void audio_play(void) {
  SDL_PauseAudio(0);
}

void audio_pause(void) {
  SDL_PauseAudio(1);
}

void audio_seek(unsigned where) {
  /* where: position in 1/100ths of a second */
  where = where * 441 * 2; /* two channels, 441MHz */
  where = where /* / 1024 * 1024*/; /* ensure it's a multiple of 1024 */
  SDL_LockAudio();
  audio->sample->dpos = where;
  SDL_UnlockAudio();
  DBPRINTF("seeking to %d", where);
}

void mix(UNUSED_PARAM(void* unused), Uint8* stream, int length) {
  Uint32 amount;
  sample_t sample = audio->sample;
  amount = sample->dlen - sample->dpos;
  if (amount > (Uint32)length) {
    amount = (Uint32)length;
  }
  SDL_MixAudio(stream, &sample->data[sample->dpos], amount, SDL_MIX_MAXVOLUME);
  sample->dpos += amount;
}

