#include <thunder/platform/pulseaudio/pulseaudio_sound_driver.h>

#include <clog/clog.h>
#include <thunder/sound_buffer.h>


#include <pulse/error.h>
#include <pulse/pulseaudio.h>
#include <pulse/sample.h>

static void startPlayback(thunder_pulseaudio_sound_driver* self)
{
    const char* defaultDevice = 0;
    const pa_cvolume* defaultVolume = 0;
    pa_stream* defaultSyncStream = 0;
    pa_stream_flags_t flags = 0;

    const int STEREO_CHANNEL_COUNT = 2;
    const int targetBufferOctetSize = 48000 * sizeof(uint16_t) * STEREO_CHANNEL_COUNT / 60;
    const int maximumBufferOctetSize = targetBufferOctetSize * 2;

    pa_buffer_attr bufferAttribute;
    tc_mem_clear_type(&bufferAttribute);

    bufferAttribute.maxlength = maximumBufferOctetSize;
    bufferAttribute.tlength = targetBufferOctetSize;
    bufferAttribute.prebuf = -1;
    bufferAttribute.minreq = -1;   // Minimum buffer requested by the server.
    bufferAttribute.fragsize = -1; // Only for recording

    flags |= PA_STREAM_ADJUST_LATENCY;

    int error = pa_stream_connect_playback(self->stream, defaultDevice, &bufferAttribute, flags, defaultVolume,
                                           defaultSyncStream);
    if (error != 0) {
        CLOG_INFO("pa_stream_connect_playback error: %d", error)
    }
}

static void writeCallback(pa_stream* stream, size_t octetLength, void* userdata)
{
    thunder_pulseaudio_sound_driver* self = (thunder_pulseaudio_sound_driver*) userdata;

    int combinedSampleOctets = sizeof(ThunderSampleOutputS16);

    int requiredSampleCount = octetLength / combinedSampleOctets;

    // printf("required sample count:%d\n", requiredSampleCount);

#define bufferLength (48000)

    ThunderSampleOutputS16 buffer[bufferLength];

    int convertedOctetCount = requiredSampleCount * combinedSampleOctets;

    thunder_audio_buffer_read(self->buffer, buffer, requiredSampleCount);

    if (pa_stream_write(stream, (uint8_t*) buffer, convertedOctetCount, 0, 0, PA_SEEK_RELATIVE) < 0) {
        CLOG_ERROR("pa_stream_write() failed: %s\n", pa_strerror(pa_context_errno(self->context)))
        return;
    }
}

static void connectingCallback(thunder_pulseaudio_sound_driver* self)
{
    CLOG_OUTPUT_STDERR("connecting...");
}

static void contextReadyCallback(thunder_pulseaudio_sound_driver* self)
{
    CLOG_OUTPUT_STDERR("context ready!");
    pa_stream* stream;
    const char* streamName = "Turmoil Sound";

    pa_sample_spec sampleSpec;

    sampleSpec.format = PA_SAMPLE_S16LE;
    sampleSpec.channels = 2;
    sampleSpec.rate = 48000;

    pa_channel_map channelMap;
    pa_channel_map_init_stereo(&channelMap);

    if (!(stream = pa_stream_new(self->context, streamName, &sampleSpec, &channelMap))) {
        CLOG_ERROR("pa_stream_new() failed: %s", pa_strerror(pa_context_errno(self->context)));
    }

    self->stream = stream;
    pa_stream_set_write_callback(self->stream, writeCallback, self);
    startPlayback(self);
}

static void stateCallback(pa_context* c, void* userdata)
{
    thunder_pulseaudio_sound_driver* self = (thunder_pulseaudio_sound_driver*) userdata;

    pa_context_state_t state = pa_context_get_state(c);
    switch (state) {
        case PA_CONTEXT_READY:
            contextReadyCallback(self);
            break;
        case PA_CONTEXT_CONNECTING:
            connectingCallback(self);
        case PA_CONTEXT_TERMINATED:
            CLOG_OUTPUT_STDERR("pulse audio state: terminating")
            break;
        default:
            CLOG_OUTPUT_STDERR("unknown pulse audio state: %d", state);
            break;
    }
}

int thunder_pulseaudio_sound_driver_init(thunder_pulseaudio_sound_driver* self, thunder_audio_buffer* buffer,
                                          bool use_floats)
{
    self->buffer = buffer;

    pa_threaded_mainloop* mainloop;

    if (!(mainloop = pa_threaded_mainloop_new())) {
        CLOG_ERROR("mainloop")
        return -2;
    }
    self->mainloop = mainloop;

    pa_mainloop_api* mainloopApi;
    mainloopApi = pa_threaded_mainloop_get_api(mainloop);

    const char* clientName = "client";

    pa_context* context;
    if (!(context = pa_context_new(mainloopApi, clientName))) {
        CLOG_ERROR("mainloop")
    }

    self->context = context;

    pa_context_set_state_callback(context, stateCallback, self);

    const char* serverHostname = 0; // default
    int connectResult = pa_context_connect(context, serverHostname, 0, 0);
    if (connectResult != 0) {
        CLOG_ERROR("can not context")
        return connectResult;
    }

    return pa_threaded_mainloop_start(mainloop);
}

void thunder_pulseaudio_sound_driver_free(thunder_pulseaudio_sound_driver* self)
{
    CLOG_OUTPUT_STDERR("pulseaudio: closing")
    pa_threaded_mainloop_stop(self->mainloop);
    pa_context_disconnect(self->context);
    pa_threaded_mainloop_free(self->mainloop);
}
