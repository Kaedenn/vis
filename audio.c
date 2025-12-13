
#include "audio.h"
#include "helper.h"

#include "miniaudio.h"

struct audio {
    char* file;
    BOOL muted;
    ma_engine engine;
    ma_sound sound;
};

static struct audio* audio = NULL;

BOOL audio_init(void) {
    ma_result result;

    if (audio != NULL) {
        eprintf("Attempt to call audio_init more than once!");
        return TRUE;
    }
    audio = DBMALLOC(sizeof(struct audio));
    audio->file = NULL;
    audio->muted = FALSE;

    result = ma_engine_init(NULL, &audio->engine);
    if (result != MA_SUCCESS) {
        EPRINTF("ma_engine_init failed with code %d", result);
        return FALSE;
    }
    return TRUE;
}

BOOL audio_open(const char* file) {
    ma_result result;

    if (!audio) {
        if (!audio_init()) {
            EPRINTF("Failed to play %s", file);
            return FALSE;
        }
    }

    if (audio->file) {
        ma_sound_uninit(&audio->sound);
        DBFREE(audio->file);
    }
    result = ma_sound_init_from_file(&audio->engine, file, 0, NULL, NULL, &audio->sound);
    if (result != MA_SUCCESS) {
        EPRINTF("ma_engine_init failed with code %d", result);
        return FALSE;
    }
    audio->file = dupstr(file);
    return TRUE;
}

void audio_free(UNUSED_PARAM(void* ptr)) {
    if (audio) {
        if (audio->file) {
            ma_sound_uninit(&audio->sound);
            DBFREE(audio->file);
            audio->file = NULL;
        }
        ma_engine_uninit(&audio->engine);
        DBFREE(audio);
        audio = NULL;
    }
}

void audio_play(void) {
    if (!ma_sound_is_playing(&audio->sound)) {
        ma_sound_start(&audio->sound);
    }
}

void audio_pause(void) {
    if (ma_sound_is_playing(&audio->sound)) {
        ma_sound_stop(&audio->sound);
    }
}

void audio_mute(void) {
    audio->muted = !audio->muted;
    if (audio->muted) {
        ma_sound_set_volume(&audio->sound, 0);
    } else {
        ma_sound_set_volume(&audio->sound, 1);
    }
}

void audio_seek(unsigned where) {
    DBPRINTF("Seeking to %u", where);
    BOOL restart = FALSE;
    if (ma_sound_is_playing(&audio->sound)) {
        ma_sound_stop(&audio->sound);
        restart = TRUE;
    }
    ma_sound_set_start_time_in_milliseconds(&audio->sound, where);
    if (restart) {
        ma_sound_start(&audio->sound);
    }
}
