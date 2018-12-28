/*

MIT License

Copyright (c) 2012 Peter Bjorklund

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
#include <clog/clog.h>
#include <thunder/platform/webassembly/sound_driver.h>
#include <thunder/sound_buffer.h>
#include <tyran/tyran_types.h>

#include <SDL2/SDL.h>

thunder_sample temp_buf[16 * 1024];

static void fill_buffer_callback(void* _self, Uint8* target, int octet_length)
{
	thunder_sound_driver* self = _self;

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

static void start_playback(thunder_sound_driver* self)
{
	SDL_PauseAudioDevice(self->device_handle, 0);
}

void audio_format(SDL_AudioSpec* want)
{
	SDL_memset(want, 0, sizeof(*want));
	want->freq = 44100;
	want->format = AUDIO_F32;
	want->channels = 2;
	want->samples = 2048;
	want->callback = fill_buffer_callback;
}

void thunder_sound_driver_init(thunder_sound_driver* self, thunder_audio_buffer* buffer)
{
	self->buffer = buffer;

	SDL_AudioSpec want, have;
	audio_format(&want);
	want.userdata = self;
	SDL_AudioDeviceID result = SDL_OpenAudioDevice(0, 0, &want, &have, 0);
	if (result <= 0) {
		CLOG_WARN("FAILED TO OPEN AUDIO!");
		return;
	}
	self->device_handle = result;
	if (have.format != want.format) {
		CLOG_WARN("We didn't get what we wanted.%d", have.format);
		return;
	}
	CLOG_VERBOSE("Starting playback:%d", have.format);
	CLOG_VERBOSE("Starting freq:%d", have.freq);
	start_playback(self);
}

void thunder_sound_driver_free(thunder_sound_driver* self)
{
	SDL_CloseAudio();
}
