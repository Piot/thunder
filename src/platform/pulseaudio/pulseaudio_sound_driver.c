#include <thunder/platform/pulseaudio/pulseaudio_sound_driver.h>

#include <clog/clog.h>
#include <thunder/sound_buffer.h>
#include <tyran/tyran_types.h>

#include <pulse/error.h>
#include <pulse/sample.h>
#include <pulse/pulseaudio.h>

thunder_sample temp_buf[16 * 1024];

/*
static void fill_f32_buffer_callback(void* _self, uint8_t* target, int octet_length)
{
	thunder_pulseaudio_sound_driver* self = _self;

	thunder_audio_buffer* sound_buffer = self->buffer;

	if ((octet_length % 4) != 0) {
		CLOG_ERROR("ERROR!!!!! Wrong octet_length");
	}
	size_t sample_count_to_fill = octet_length / sizeof(float);
	if (sample_count_to_fill > 16 * 1024) {
		CLOG_ERROR("ERROR!!!!! sample_count too big");
		return;
	}
	if ((sample_count_to_fill % 8) != 0) {
		CLOG_ERROR("DIVISON ERR");
		return;
	}

	float* float_target = (float*) target;
	thunder_audio_buffer_read(sound_buffer, temp_buf, sample_count_to_fill);
	size_t spin = sample_count_to_fill / 8;
	thunder_sample* source = temp_buf;
	const float divisor = 33000.0f;
	for (size_t i = 0; i < spin; ++i) {
		*float_target++ = *source++ / divisor;
		*float_target++ = *source++ / divisor;
		*float_target++ = *source++ / divisor;
		*float_target++ = *source++ / divisor;
		*float_target++ = *source++ / divisor;
		*float_target++ = *source++ / divisor;
		*float_target++ = *source++ / divisor;
		*float_target++ = *source++ / divisor;
	}
}
 */

static void fill_s16_buffer_callback(void* _self, uint8_t* target, int octet_length)
{
	thunder_pulseaudio_sound_driver* self = _self;

	thunder_audio_buffer* sound_buffer = self->buffer;

	if ((octet_length % 4) != 0) {
		CLOG_ERROR("ERROR!!!!! Wrong octet_length");
	}
	size_t sample_count_to_fill = octet_length / sizeof(s16t);
	if (sample_count_to_fill > 16 * 1024) {
		CLOG_ERROR("ERROR!!!!! sample_count too big");
		return;
	}
	if ((sample_count_to_fill % 8) != 0) {
		CLOG_ERROR("DIVISON ERR");
		return;
	}

	thunder_sample_output_s16* int_target = (thunder_sample_output_s16*) target;
	thunder_audio_buffer_read(sound_buffer, int_target, sample_count_to_fill);
}

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

static void writeCallback(pa_stream *stream, size_t length, void *userdata) {
	thunder_pulseaudio_sound_driver* self = (thunder_pulseaudio_sound_driver*) userdata;

	const size_t bufferLength = 44100;
	uint8_t buffer[bufferLength];

	size_t l = length;
	if (l > bufferLength) {
		l = bufferLength;
	}

	int sampleCount = l/(sizeof(thunder_sample_output_s16)*2);

	thunder_audio_buffer_read(self->buffer, buffer, sampleCount);

	if (pa_stream_write(stream, (uint8_t*) buffer + 0, l, NULL, 0, PA_SEEK_RELATIVE) < 0) {
		CLOG_ERROR( "pa_stream_write() failed: %s\n", pa_strerror(pa_context_errno(self->context)));
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
	sampleSpec.rate = 44100;


	if (!(stream = pa_stream_new(self->context, streamName, &sampleSpec, defaultChannelMap))) {
		CLOG_ERROR("pa_stream_new() failed: %s", pa_strerror(pa_context_errno(self->context)));
	}


	self->stream = stream;
	pa_stream_set_write_callback(self->stream, writeCallback, self);
	startPlayback(self);
}

static void stateCallback(pa_context *c, void *userdata)
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
	ss.rate = 44100;

	pa_stream *stream = 0;

	pa_mainloop* mainloop;

	if (!(mainloop = pa_mainloop_new())) {
		CLOG_ERROR("mainloop")
		return;
	}
	self->mainloop = mainloop;

	pa_mainloop_api* mainloopApi;
	mainloopApi = pa_mainloop_get_api(mainloop);

	const char* clientName = "client";

	pa_context *context;
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

}

void thunder_pulseaudio_sound_driver_update(thunder_pulseaudio_sound_driver* self)
{
	int returnValue;
	pa_mainloop_iterate(self->mainloop, 1, &returnValue);
}

void thunder_pulseaudio_sound_driver_free(thunder_pulseaudio_sound_driver* self)
{
}
