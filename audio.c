
#include "audio.h"
#include "helper.h"

#include <SDL.h>
#include <SDL_audio.h>

static void mix(void*, Uint8* stream, int length);

typedef struct sample {
    Uint8* data;
    Uint32 dpos;
    Uint32 dlen;
} *sample_t;

typedef struct audio {
    char* file;
    BOOL muted;
    sample_t sample;
} *audio_t;

static audio_t audio = NULL;

BOOL audio_init(void) {
    SDL_AudioSpec fmt;
    SDL_zero(fmt);
    
    /* 16-bit stereo at 22kHz */
    fmt.freq = VIS_AUDIO_FREQ;
    fmt.format = AUDIO_S16;
    fmt.channels = VIS_AUDIO_CHANNELS;
    fmt.samples = VIS_AUDIO_SAMPLES;
    fmt.callback = mix;
    fmt.userdata = NULL;
    
    if (SDL_OpenAudio(&fmt, NULL) < 0) {
        EPRINTF("Unable to open audio: %s\n", SDL_GetError());
        return FALSE;
    }
    
    audio = DBMALLOC(sizeof(struct audio));
    audio->file = NULL;
    audio->muted = FALSE;
    audio->sample = DBMALLOC(sizeof(struct sample));
    audio->sample->data = NULL;
    audio->sample->dpos = 0;
    audio->sample->dlen = 0;
    return TRUE;
}

void audio_free(UNUSED_PARAM(void* arg)) {
    if (audio) {
        DBFREE(audio->file);
        DBFREE(audio->sample->data);
        DBFREE(audio->sample);
        DBFREE(audio);
    }
    SDL_CloseAudio();
}

BOOL audio_open(const char* file) {
    SDL_AudioSpec wave;
    Uint8 *data;
    Uint32 dlen;
    SDL_AudioCVT cvt;
    
    if (audio == NULL) {
        if (!audio_init()) {
            return FALSE;
        }
    }
    if (audio->file != NULL) {
        free(audio->file);
    }
    audio->file = dupstr(file);
    
    /* Load the sound file and convert it to 16-bit stereo at 22kHz */
    if (SDL_LoadWAV(file, &wave, &data, &dlen) == NULL) {
        EPRINTF("Couldn't load %s: %s\n", file, SDL_GetError());
        return FALSE;
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
        DBFREE(audio->sample->data);
    }
    SDL_LockAudio();
    audio->sample->data = cvt.buf;
    audio->sample->dlen = (Uint32)cvt.len_cvt;
    audio->sample->dpos = 0;
    SDL_UnlockAudio();
    return TRUE;
}

void audio_close(void) {
    DBFREE(audio->file);
    DBFREE(audio);
    audio = NULL;
    SDL_AudioQuit();
}

void audio_mute(void) {
    audio->muted = TRUE;
}

void audio_play(void) {
    SDL_PauseAudio(0);
}

void audio_pause(void) {
    SDL_PauseAudio(1);
}

void audio_seek(unsigned where) {
    /* where: position in 1/100ths of a second */
    where = where * VIS_AUDIO_FREQ/100 * 2;
    SDL_LockAudio();
    audio->sample->dpos = where;
    SDL_UnlockAudio();
    DBPRINTF("seeking to %d", where);
}

void mix(UNUSED_PARAM(void* unused), Uint8* stream, int length) {
    SDL_memset(stream, 0, (size_t)length);
    Uint32 amount;
    sample_t sample = audio->sample;
    amount = sample->dlen - sample->dpos;
    if (amount > (Uint32)length) {
        amount = (Uint32)length;
    }
    SDL_MixAudio(stream, &sample->data[sample->dpos], amount,
                 audio->muted ? 0 : SDL_MIX_MAXVOLUME);
    sample->dpos += amount;
}

