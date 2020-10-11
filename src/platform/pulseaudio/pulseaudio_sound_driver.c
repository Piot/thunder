#include <thunder/platform/pulseaudio/pulseaudio_sound_driver.h>

#include <clog/clog.h>
#include <thunder/sound_buffer.h>
#include <tyran/tyran_types.h>

#include <pulse/error.h>
#include <pulse/sample.h>
#include <pulse/simple.h>

thunder_sample temp_buf[16 * 1024];

static void fill_f32_buffer_callback(void* _self, Uint8* target, int octet_length)
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

static void fill_s16_buffer_callback(void* _self, Uint8* target, int octet_length)
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
}

static void logLatency(pa_simple* simple)
{
	pa_usec_t latency;
	int pulseAudioError;
	if ((latency = pa_simple_get_latency(simple, &pulseAudioError)) == (pa_usec_t) -1) {
		CLOG_SOFT_ERROR("pa_simple_get_latency() failed: %s", pa_strerror(pulseAudioError));
		return;
	}
	CLOG_INFO("%0.0f usec", (float) latency);
}

void thunder_pulseaudio_sound_driver_init(thunder_pulseaudio_sound_driver* self, thunder_audio_buffer* buffer, tyran_boolean use_floats)
{
	self->buffer = buffer;

	pa_sample_spec ss;
	ss.format = PA_SAMPLE_S16NE;
	ss.channels = 2;
	ss.rate = 44100;

	int pulseAudioError;
	pa_simple* simple = pa_simple_new(0,		 // Use the default server.
									  "Turmoil", // Our application's name.
									  PA_STREAM_PLAYBACK,
									  0,	  // Use the default device.
									  "Game", // Description of our stream.
									  &ss,	  // Our sample format.
									  0,	  // Use default channel map
									  0,	  // Use default buffering attributes.
									  &pulseAudioError);

	if (!simple) {
		CLOG_SOFT_ERROR("pulseaudio: %s", pa_strerror(pulseAudioError));
		return;
	}

	self->simple = simple;

	logLatency(self->simple);

	startPlayback(self);
}

void thunder_pulseaudio_sound_driver_update(thunder_pulseaudio_sound_driver* self)
{
	thunder_sample_output_s16 temp[1024];

	int availableSampleCount = self->buffer->read_buffer_samples_left;
	if (availableSampleCount > 1024) {
		availableSampleCount = 1024;
	}
	thunder_audio_buffer_read(self->buffer, temp, availableSampleCount);

	int simpleError;
	int result = pa_simple_write(self->simple, temp, sizeof(thunder_sample_output_s16) * availableSampleCount, &simpleError);
	if (!result) {
		CLOG_SOFT_ERROR("write error");
	}
}

void thunder_pulseaudio_sound_driver_free(thunder_pulseaudio_sound_driver* self)
{
}
