
#ifndef VIS_PA_LATENCY_PROBE_HEADER_INCLUDED_
#define VIS_PA_LATENCY_PROBE_HEADER_INCLUDED_ 1

#include <stdint.h>

typedef struct pa_latency_probe pa_latency_probe;

pa_latency_probe* pa_latency_probe_create(uint32_t rate, uint8_t channels);

void pa_latency_probe_destroy(pa_latency_probe* probe);

uint64_t pa_latency_probe_get_delay_ms(pa_latency_probe* probe);

#endif
