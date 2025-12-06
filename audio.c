
#include "audio.h"
#include "helper.h"

#include <SDL.h>
#include <SDL_mixer.h>

struct audio {
    char* file;
    BOOL muted;
    Mix_Music* music;
};

static struct audio* audio = NULL;

BOOL audio_init(void) {
    return FALSE;
    /*
    if (audio != NULL) {
        eprintf("Attempt to call audio_init more than once!");
        return TRUE;
    }
    if (Mix_OpenAudio(VIS_AUDIO_FREQ, MIX_DEFAULT_FORMAT, 2,
                      VIS_AUDIO_SAMPLES) < 0) {
        EPRINTF("Unable to start audio engine: %s", Mix_GetError());
        return FALSE;
    }

    audio = DBMALLOC(sizeof(struct audio));
    return TRUE;
    */
}

BOOL audio_open(const char* file) {
    return FALSE;
    /*
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
    */
}

void audio_free(UNUSED_PARAM(void* ptr)) {
    /*
    audio_close();
    DBFREE(audio);
    audio = NULL;
    */
}

void audio_close(void) {
    /*
    Mix_FreeMusic(audio->music);
    audio->music = NULL;
    DBFREE(audio->file);
    audio->file = NULL;
    Mix_CloseAudio();
    while (Mix_Init(0)) {
        Mix_Quit();
    }
    */
}

void audio_play(void) {
    /*
    if (Mix_PausedMusic() == 1) {
        Mix_ResumeMusic();
    }
    */
}

void audio_pause(void) {
    /*
    if (Mix_PausedMusic() == 0) {
        Mix_PauseMusic();
    }
    */
}

void audio_mute(void) {
    /*
    Mix_VolumeMusic(0);
    */
}

void audio_seek(unsigned where) {
    /*
    DBPRINTF("Seeking to %u", where);
    Mix_RewindMusic();
    if (Mix_SetMusicPosition((double)where / 100) == -1) {
        EPRINTF("Codec does not support seeking: %s", Mix_GetError());
    }
    */
}

