
#ifndef VIS_AUDIO_HEADER_INCLUDED_
#define VIS_AUDIO_HEADER_INCLUDED_ 1

#include "helper.h"

void audio_init(void);

BOOL audio_open(const char* file);
void audio_free(void* ptr);
void audio_close(void);

void audio_play(void);
void audio_pause(void);
void audio_seek(unsigned where);

#endif

