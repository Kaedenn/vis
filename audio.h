
#ifndef VIS_AUDIO_HEADER_INCLUDED_
#define VIS_AUDIO_HEADER_INCLUDED_ 1

#include "defines.h"

BOOL audio_init(void);

BOOL audio_open(const char* file);
void audio_free(void* ptr);

void audio_play(void);
void audio_pause(void);
BOOL audio_is_playing(void);

/* mutes the sound's audio */
void audio_mute(void);

/* change the master volume; volume must be between 0 and 1 */
void audio_set_volume(float volume);

/* seek offset is in milliseconds */
void audio_seek(unsigned where);

#endif
