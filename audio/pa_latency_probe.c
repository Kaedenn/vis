
#include "../defines.h"
#include "../helper.h"
#include "pa_latency_probe.h"

#ifdef HAVE_PULSEAUDIO

#include <pulse/pulseaudio.h>
#include <stdlib.h>
#include <string.h>

struct pa_latency_probe {
    pa_threaded_mainloop* ml;
    pa_context*           ctx;
    pa_stream*            stream;
    pa_sample_spec        ss;

    int                   ready;
    size_t                frame_size;
};

static void signal_ml(pa_latency_probe* p) {
    if (p && p->ml) pa_threaded_mainloop_signal(p->ml, 0);
}

static void ctx_state_cb(UNUSED_PARAM(pa_context* c), void* userdata) {
    signal_ml((pa_latency_probe*)userdata);
}

static void stream_state_cb(UNUSED_PARAM(pa_stream* s), void* userdata) {
    signal_ml((pa_latency_probe*)userdata);
}

static void timing_cb(UNUSED_PARAM(pa_stream* s), UNUSED_PARAM(int success), void* userdata) {
    signal_ml((pa_latency_probe*)userdata);
}

// Keep the stream running by writing silence whenever PA asks for data.
static void write_cb(pa_stream* s, size_t nbytes, void* userdata) {
    pa_latency_probe* p = (pa_latency_probe*)userdata;
    if (!p || !s || nbytes == 0) return;

    // Allocate a temporary zero buffer (silence).
    // For efficiency you could keep a static buffer, but this is simple.
    void* buf = calloc(1, nbytes);
    if (!buf) return;

    // Write silence; PA will mix it (it’s just zeros).
    // The stream will remain “running”, enabling meaningful timing/latency.
    pa_stream_write(s, buf, nbytes, free, 0, PA_SEEK_RELATIVE);
}

static int wait_for_context_ready(pa_latency_probe* p) {
    for (;;) {
        pa_context_state_t st = pa_context_get_state(p->ctx);
        if (st == PA_CONTEXT_READY) return 0;
        if (!PA_CONTEXT_IS_GOOD(st)) return -1;
        pa_threaded_mainloop_wait(p->ml);
    }
}

static int wait_for_stream_ready(pa_latency_probe* p) {
    for (;;) {
        pa_stream_state_t st = pa_stream_get_state(p->stream);
        if (st == PA_STREAM_READY) return 0;
        if (!PA_STREAM_IS_GOOD(st)) return -1;
        pa_threaded_mainloop_wait(p->ml);
    }
}

pa_latency_probe* pa_latency_probe_create(uint32_t rate, uint8_t channels) {
    pa_latency_probe* probe = DBMALLOC(sizeof(pa_latency_probe));
    probe->ml = pa_threaded_mainloop_new();
    if (!probe->ml) {
        EPRINTF("Failed to allocate threading mainloop");
        pa_latency_probe_destroy(probe);
        return NULL;
    }

    pa_mainloop_api* api = pa_threaded_mainloop_get_api(probe->ml);
    probe->ctx = pa_context_new(api, "latency-probe");
    if (!probe->ctx) {
        EPRINTF("Failed to allocate latency probe context");
        pa_latency_probe_destroy(probe);
        return NULL;
    }

    pa_context_set_state_callback(probe->ctx, ctx_state_cb, probe);

    probe->ss.format = PA_SAMPLE_FLOAT32LE;
    probe->ss.rate = rate ? rate : 48000;
    probe->ss.channels = channels ? channels : 2;

    probe->frame_size = (size_t)probe->ss.channels * sizeof(float);

    if (pa_threaded_mainloop_start(probe->ml) < 0) {
        EPRINTF("Failed to start threading mainloop");
        pa_latency_probe_destroy(probe);
        return NULL;
    }
    pa_threaded_mainloop_lock(probe->ml);

    if (pa_context_connect(probe->ctx, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
        EPRINTF("Failed to connect context");
        pa_latency_probe_destroy(probe);
        return NULL;
    }
    if (wait_for_context_ready(probe) != 0) {
        pa_threaded_mainloop_unlock(probe->ml);
        EPRINTF("wait_for_context_ready failed");
        pa_latency_probe_destroy(probe);
        return NULL;
    }

    probe->stream = pa_stream_new(probe->ctx, "latency-probe-stream", &probe->ss, NULL);
    if (!probe->stream) {
        EPRINTF("Failed to allocate stream");
        pa_latency_probe_destroy(probe);
        return NULL;
    }

    pa_stream_set_state_callback(probe->stream, stream_state_cb, probe);
    pa_stream_set_write_callback(probe->stream, write_cb, probe);

    pa_stream_flags_t flags = 
        PA_STREAM_INTERPOLATE_TIMING |
        PA_STREAM_AUTO_TIMING_UPDATE;

    // Default sink (NULL) so it follows whatever your system/miniaudio uses.
    if (pa_stream_connect_playback(probe->stream, NULL, NULL, flags, NULL, NULL) < 0) {
        EPRINTF("Failed to connect to stream for playback");
        pa_latency_probe_destroy(probe);
        return NULL;
    }
    if (wait_for_stream_ready(probe) != 0) {
        EPRINTF("wait_for_stream_ready failure");
        pa_threaded_mainloop_unlock(probe->ml);
        pa_latency_probe_destroy(probe);
        return NULL;
    }

    // Kick the stream by writing a small chunk of silence immediately.
    // (Some setups won’t call write_cb until you’ve started feeding.)
    {
        const size_t kick = probe->frame_size * 256; // ~256 frames of silence
        void* buf = DBMALLOC(kick);
        if (buf) pa_stream_write(probe->stream, buf, kick, free, 0, PA_SEEK_RELATIVE);
    }

    pa_threaded_mainloop_unlock(probe->ml);
    return probe;
}

void pa_latency_probe_destroy(pa_latency_probe* probe) {
    if (!probe) return;

    if (probe->ml) pa_threaded_mainloop_lock(probe->ml);

    if (probe->stream) {
        pa_stream_disconnect(probe->stream);
        pa_stream_unref(probe->stream);
        probe->stream = NULL;
    }

    if (probe->ctx) {
        pa_context_disconnect(probe->ctx);
        pa_context_unref(probe->ctx);
        probe->ctx = NULL;
    }

    if (probe->ml) {
        pa_threaded_mainloop_unlock(probe->ml);
        pa_threaded_mainloop_stop(probe->ml);
        pa_threaded_mainloop_free(probe->ml);
        probe->ml = NULL;
    }

    DBFREE(probe);
}

uint64_t pa_latency_probe_get_delay_ms(pa_latency_probe* probe) {
    if (!probe || !probe->ml || !probe->stream) {
        EPRINTF("Calling pa_latency_probe_delay_ms without constructed probe");
        return 0;
    }

    pa_threaded_mainloop_lock(probe->ml);
    pa_operation* op = pa_stream_update_timing_info(probe->stream, timing_cb, probe);
    if (op) pa_operation_unref(op);
    pa_threaded_mainloop_wait(probe->ml);

    pa_usec_t usec = 0;
    int negative = 0;
    if (pa_stream_get_latency(probe->stream, &usec, &negative) < 0) {
        EPRINTF("Failed to get stream latency");
        pa_threaded_mainloop_unlock(probe->ml);
        return 0;
    }

    if (negative) usec = 0;
    pa_threaded_mainloop_unlock(probe->ml);

    return (uint64_t)usec / 1000;
}

#else

/* ISO C forbids empty translation unit */
uint64_t pa_latency_probe_get_delay_ms(void* probe) { return 0; }

#endif
