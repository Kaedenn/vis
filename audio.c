
#include "audio.h"
#include "helper.h"

#include <SDL.h>
#include <SDL_mixer.h>

#if 0
static void mix(void*, Uint8* stream, int length);

struct sample {
    Uint8* data;
    Uint32 dpos;
    Uint32 dlen;
};
#endif

struct audio {
    char* file;
    BOOL muted;
    Mix_Music* music;
};

static struct audio* audio = NULL;

#if 1
BOOL audio_init(void) {
    if (Mix_OpenAudio(VIS_AUDIO_FREQ, MIX_DEFAULT_FORMAT, 2,
                      VIS_AUDIO_SAMPLES) < 0) {
        EPRINTF("Unable to start audio engine: %s", Mix_GetError());
        return FALSE;
    }

    audio = DBMALLOC(sizeof(struct audio));
    return TRUE;
}

BOOL audio_open(const char* file) {
    if (!audio) {
        if (!audio_init()) {
            EPRINTF("Failed to play %s", file);
            return FALSE;
        }
    }

    if ((audio->music = Mix_LoadMUS(file)) == NULL) {
        EPRINTF("Failed to load %s: %s", file, Mix_GetError());
        return FALSE;
    }

    if (Mix_PlayMusic(audio->music, -1) == -1) {
        EPRINTF("Failed to play %s: %s", file, Mix_GetError());
        Mix_FreeMusic(audio->music);
        return FALSE;
    }

    audio->file = dupstr(file);
    Mix_PauseMusic();
    return TRUE;
}

void audio_free(UNUSED_PARAM(void* ptr)) {
    audio_close();
    DBFREE(audio);
    audio = NULL;
}

void audio_close(void) {
    Mix_FreeMusic(audio->music);
    audio->music = NULL;
    DBFREE(audio->file);
    audio->file = NULL;
}

void audio_play(void) {
    if (Mix_PausedMusic() == 1) {
        Mix_ResumeMusic();
    }
}

void audio_pause(void) {
    if (Mix_PausedMusic() == 0) {
        Mix_PauseMusic();
    }
}

void audio_mute(void) {
    Mix_VolumeMusic(0);
}

void audio_seek(unsigned where) {
    DBPRINTF("Seeking to %u", where);
    Mix_RewindMusic();
    if (Mix_SetMusicPosition((double)where / 100) == -1) {
        EPRINTF("Codec does not support seeking: %s", Mix_GetError());
    }
}

#else

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
        DBFREE(audio->file);
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
    cvt.buf = DBMALLOC(dlen * (Uint32)cvt.len_mult);
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
    where = where * 220500/100 * 2;
    SDL_LockAudio();
    audio->sample->dpos = where;
    SDL_UnlockAudio();
    DBPRINTF("seeking to %d", where);
}

void mix(UNUSED_PARAM(void* unused), Uint8* stream, int length) {
    SDL_memset(stream, 0, (size_t)length);
    Uint32 amount;
    struct sample* sample = audio->sample;
    amount = sample->dlen - sample->dpos;
    if (amount > (Uint32)length) {
        amount = (Uint32)length;
    }
    SDL_MixAudio(stream, &sample->data[sample->dpos], amount,
                 audio->muted ? 0 : SDL_MIX_MAXVOLUME);
    sample->dpos += amount;
}
#endif

