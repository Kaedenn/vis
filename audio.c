
#include "audio.h"
#include "helper.h"

#include "miniaudio.h"

struct audio {
    char* file;
    BOOL muted;
    ma_engine* engine;
    ma_sound* sound;
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
    audio->engine = DBMALLOC(sizeof(*audio->engine));
    audio->sound = NULL;

    ma_engine_config config;
    config = ma_engine_config_init();
    config.noDevice = MA_FALSE;
    config.channels = VIS_AUDIO_CHANNELS;
    config.sampleRate = VIS_AUDIO_FREQ;

    result = ma_engine_init(&config, audio->engine);
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
        DBFREE(audio->file);
        audio->file = NULL;
    }
    if (audio->sound) {
        ma_sound_uninit(audio->sound);
        /* leave audio->sound allocated; we're going to reuise it */
    } else {
        audio->sound = DBMALLOC(sizeof(*audio->sound));
    }
    result = ma_sound_init_from_file(
        audio->engine, file, MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, audio->sound);
    if (result != MA_SUCCESS) {
        EPRINTF("ma_sound_init_from_file failed with code %d", result);
        return FALSE;
    }
    audio->file = dupstr(file);
    return TRUE;
}

void audio_free(UNUSED_PARAM(void* ptr)) {
    if (audio) {
        if (audio->file) {
            DBFREE(audio->file);
            audio->file = NULL;
        }
        if (audio->sound) {
            ma_sound_uninit(audio->sound);
            DBFREE(audio->sound);
            audio->sound = NULL;
        }
        ma_engine_uninit(audio->engine);
        DBFREE(audio->engine);
        DBFREE(audio);
        audio = NULL;
    }
}

void audio_play(void) {
    if (!ma_sound_is_playing(audio->sound)) {
        ma_sound_start(audio->sound);
    }
}

void audio_pause(void) {
    if (ma_sound_is_playing(audio->sound)) {
        ma_sound_stop(audio->sound);
    }
}

BOOL audio_is_playing(void) {
    return (BOOL)ma_sound_is_playing(audio->sound);
}

void audio_mute(void) {
    audio->muted = !audio->muted;
    if (audio->muted) {
        ma_sound_set_volume(audio->sound, 0.0f);
    } else {
        ma_sound_set_volume(audio->sound, 1.0f);
    }
}

void audio_set_volume(float volume) {
    ma_device* device = ma_engine_get_device(audio->engine);
    if (device != NULL) {
        ma_device_set_master_volume(device, volume);
    }
}

void audio_seek(unsigned where) {
    BOOL restart = FALSE;
    if (ma_sound_is_playing(audio->sound)) {
        ma_sound_stop(audio->sound);
        restart = TRUE;
    }
    ma_sound_set_start_time_in_milliseconds(audio->sound, where);
    if (restart) {
        ma_sound_start(audio->sound);
    }
}
