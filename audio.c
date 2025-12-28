
#include "audio.h"
#include "helper.h"

#include "3rdparty/miniaudio.h"

struct audio {
    char* file;
    BOOL muted;
    float volume;
    ma_context context;
    ma_engine engine;
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
    audio->volume = 1.0f;
    audio->sound = NULL;

    ma_context_config ctxConfig = ma_context_config_init();

    result = ma_context_init(NULL, 0, &ctxConfig, &audio->context);
    if (result != MA_SUCCESS) {
        EPRINTF("ma_context_init failed with code %d", result);
        return FALSE;
    }

    ma_engine_config config;
    config = ma_engine_config_init();
    config.pContext = &audio->context;
    config.noDevice = MA_FALSE;
    config.channels = VIS_AUDIO_CHANNELS;
    config.sampleRate = VIS_AUDIO_FREQ;

    result = ma_engine_init(&config, &audio->engine);
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
        DZFREE(audio->file);
    }
    if (audio->sound) {
        ma_sound_uninit(audio->sound);
        /* leave audio->sound allocated; we're going to reuse it */
    } else {
        audio->sound = DBMALLOC(sizeof(*audio->sound));
    }
    result = ma_sound_init_from_file(
        &audio->engine, file, MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, audio->sound);
    if (result != MA_SUCCESS) {
        EPRINTF("ma_sound_init_from_file failed with code %d", result);
        return FALSE;
    }
    audio->file = dupstr(file);
    return TRUE;
}

void audio_free(void) {
    if (audio) {
        if (audio->file) {
            DZFREE(audio->file);
        }
        if (audio->sound) {
            ma_sound_uninit(audio->sound);
            DZFREE(audio->sound);
        }
        ma_engine_uninit(&audio->engine);
        DZFREE(audio);
    }
}

void audio_play(void) {
    if (!audio->sound) { return; }
    if (!ma_sound_is_playing(audio->sound)) {
        ma_sound_start(audio->sound);
    }
}

void audio_pause(void) {
    if (!audio->sound) { return; }
    if (ma_sound_is_playing(audio->sound)) {
        ma_sound_stop(audio->sound);
    }
}

BOOL audio_is_playing(void) {
    if (!audio->sound) { return FALSE; }
    return (BOOL)ma_sound_is_playing(audio->sound);
}

void audio_mute(void) {
    DBPRINTF("Muting audio; volume was %g", audio->volume);
    if (!audio->sound) { return; }
    if (!audio->muted) {
        ma_sound_set_volume(audio->sound, 0.0f);
        audio->muted = TRUE;
    }
}

void audio_unmute(void) {
    DBPRINTF("Unmuting audio; volume is %g", audio->volume);
    if (!audio->sound) { return; }
    if (audio->muted) {
        ma_sound_set_volume(audio->sound, audio->volume);
        audio->muted = FALSE;
    }
}

BOOL audio_is_muted(void) {
    return audio->muted;
}

void audio_set_volume(float volume) {
    DBPRINTF("Setting volume to %g", volume);
    if (!audio->sound) { return; }
    /*ma_device* device = ma_engine_get_device(audio->engine);
    if (device != NULL) {
        ma_device_set_master_volume(device, volume);
    }*/
    ma_sound_set_volume(audio->sound, volume);
    DBPRINTF("Set volume to %g, got %g", volume, ma_sound_get_volume(audio->sound));
    audio->volume = volume;
}

void audio_seek(unsigned where) {
    DBPRINTF("Seeking audio to %u", where);
    BOOL restart = FALSE;
    if (!audio->sound) { return; }
    if (ma_sound_is_playing(audio->sound)) {
        ma_sound_stop(audio->sound);
        restart = TRUE;
    }
    ma_sound_set_start_time_in_milliseconds(audio->sound, where);
    if (restart) {
        ma_sound_start(audio->sound);
    }
}
