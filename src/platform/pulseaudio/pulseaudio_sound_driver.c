#include <thunder/platform/pulseaudio/pulseaudio_sound_driver.h>

#include <clog/clog.h>
#include <thunder/sound_buffer.h>
#include <tyran/tyran_types.h>

#include <pulse/error.h>
#include <pulse/pulseaudio.h>
#include <pulse/sample.h>

static void startPlayback(thunder_pulseaudio_sound_driver* self)
{
	const char* defaultDevice = 0;

	/*
	 *                 memset(&buffer_attr, 0, sizeof(buffer_attr));
				buffer_attr.tlength = (uint32_t) latency;
				buffer_attr.minreq = (uint32_t) process_time;
				buffer_attr.maxlength = (uint32_t) -1;
				buffer_attr.prebuf = (uint32_t) -1;
				buffer_attr.fragsize = (uint32_t) latency;
				flags |= PA_STREAM_ADJUST_LATENCY;
	 */
	pa_buffer_attr* defaultAttribute = 0;
	const pa_cvolume* defaultVolume = 0;
	pa_stream* defaultSyncStream = 0;
	pa_stream_flags_t defaultFlags = 0;

	int error = pa_stream_connect_playback(self->stream, defaultDevice, defaultAttribute, defaultFlags, defaultVolume, defaultSyncStream);
	if (error != 0) {
		CLOG_INFO("pa_stream_connect_playback error: %d", error)
	}
}

static void writeCallback(pa_stream* stream, size_t octetLength, void* userdata)
{
	thunder_pulseaudio_sound_driver* self = (thunder_pulseaudio_sound_driver*) userdata;

	int combinedSampleOctets = sizeof(thunder_sample_output_s16);

	int requiredSampleCount = octetLength / combinedSampleOctets;

	const size_t bufferLength = 44100;
	thunder_sample_output_s16 buffer[bufferLength];

	int convertedOctetCount = requiredSampleCount * combinedSampleOctets;

	thunder_audio_buffer_read(self->buffer, buffer, requiredSampleCount);

	if (pa_stream_write(stream, (uint8_t*) buffer, convertedOctetCount, NULL, 0, PA_SEEK_RELATIVE) < 0) {
		CLOG_ERROR("pa_stream_write() failed: %s\n", pa_strerror(pa_context_errno(self->context)));
		return;
	}
}

static void connectingCallback(thunder_pulseaudio_sound_driver* self)
{
	CLOG_INFO("connecting...");
}

static void contextReadyCallback(thunder_pulseaudio_sound_driver* self)
{
	CLOG_INFO("context ready!");
	pa_stream* stream;
	const pa_channel_map* defaultChannelMap = 0;
	const char* streamName = "Turmoil Sound";

	pa_sample_spec sampleSpec;

	sampleSpec.format = PA_SAMPLE_S16LE;
	sampleSpec.channels = 2;
	sampleSpec.rate = 48000;

	if (!(stream = pa_stream_new(self->context, streamName, &sampleSpec, defaultChannelMap))) {
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
		default:
			CLOG_INFO("state: %d", state);
			break;
	}
}

void thunder_pulseaudio_sound_driver_init(thunder_pulseaudio_sound_driver* self, thunder_audio_buffer* buffer, tyran_boolean use_floats)
{
	self->buffer = buffer;

	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16BE;
	ss.channels = 2;
	ss.rate = 48000;

	pa_stream* stream = 0;

	pa_threaded_mainloop* mainloop;

	if (!(mainloop = pa_threaded_mainloop_new())) {
		CLOG_ERROR("mainloop")
		return;
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
	}

	/*
		int ret;
		if (pa_mainloop_run(mainloop, &ret) < 0) {
			CLOG_INFO("pa_mainloop_run() failed.");
		}
		*/

	pa_threaded_mainloop_start(mainloop);
}

void thunder_pulseaudio_sound_driver_free(thunder_pulseaudio_sound_driver* self)
{
}
