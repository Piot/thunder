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
#include <thunder/audio_node.h>
#include <thunder/sound_buffer.h>
#include <thunder/sound_compositor.h>
#include <thunder/sound_module.h>
#include <tyran/tyran_log.h>
#include <tyran/tyran_memory.h>

static const int THUNDER_ATOM_SAMPLE_COUNT = 2 * 1024;

static void mix_down_using_volume(thunder_mix_sample* source, int size, float mix_down_volume, thunder_sample* target)
{
	const int DIVISOR = 8;
	TYRAN_ASSERT((THUNDER_ATOM_SAMPLE_COUNT % DIVISOR) == 0, "Illegal divisor");
	for (int i = 0; i < size / DIVISOR; ++i) {
		*target = *source++ * mix_down_volume;
		target++;
		*target = *source++ * mix_down_volume;
		target++;
		*target = *source++ * mix_down_volume;
		target++;
		*target = *source++ * mix_down_volume;
		target++;
		*target = *source++ * mix_down_volume;
		target++;
		*target = *source++ * mix_down_volume;
		target++;
		*target = *source++ * mix_down_volume;
		target++;
		*target = *source++ * mix_down_volume;
		target++;
	}
}

static thunder_mix_sample maximum_amplitude(const thunder_mix_sample* target, int size)
{
	thunder_mix_sample maximum_amplitude_found = 0;
	thunder_mix_sample minimum_amplitude_found = 0;
	thunder_mix_sample v;

	for (int i = 0; i < size; ++i) {
		v = *target++;

		if (v > maximum_amplitude_found) {
			maximum_amplitude_found = v;
		} else if (v < minimum_amplitude_found) {
			minimum_amplitude_found = v;
		}
	}

	thunder_mix_sample max_amplitude = (-minimum_amplitude_found > maximum_amplitude_found) ? -minimum_amplitude_found : maximum_amplitude_found;

	return max_amplitude;
}

static void adjust_mix_down_volume(thunder_audio_compositor* self, thunder_mix_sample maximum_amplitude_found)
{
	float maximum_amplitude_as_factor = (float) maximum_amplitude_found / 32767.0f;
	if (maximum_amplitude_found <= 0.01f) {
		return;
	}
	float optimal_volume = (1.0f / maximum_amplitude_as_factor);
	// We don't want to increase the volume too much
	const static float max_volume = 1.4f;
	if (optimal_volume > max_volume) {
		optimal_volume = max_volume;
	}
	if (optimal_volume < self->mix_down_volume) {
		self->mix_down_volume = optimal_volume * 0.9f;
		TYRAN_LOG_VERBOSE("HIGH AMPLITUDE ADJUSTING: Max Found:%d, Dynamics:%f", maximum_amplitude_found, self->mix_down_volume);
	} else if (optimal_volume >= self->mix_down_volume + 0.1f) { // It must be a big difference to be bother to increased the volume
		self->mix_down_volume += (optimal_volume - self->mix_down_volume) / (float) (60 * 30);
		TYRAN_LOG_VERBOSE("slightly increasing Max Found:%d, Dynamics:%f", maximum_amplitude_found, self->mix_down_volume);
	}
}

static void add_to_output(thunder_mix_sample* output, int channel, int number_of_channels, const thunder_sample* source_sample, tyran_number volume)
{
	thunder_mix_sample* target = output + channel;
	thunder_mix_sample* end = output + THUNDER_ATOM_SAMPLE_COUNT; // - channel;
	const thunder_sample* source = source_sample;

	while (target < end) {
		*target += *source++ * volume;
		target += number_of_channels;
		*target += *source++ * volume;
		target += number_of_channels;
		*target += *source++ * volume;
		target += number_of_channels;
		*target += *source++ * volume;
		target += number_of_channels;
		*target += *source++ * volume;
		target += number_of_channels;
		*target += *source++ * volume;
		target += number_of_channels;
		*target += *source++ * volume;
		target += number_of_channels;
		*target += *source++ * volume;
		target += number_of_channels;
	}
}

static void enumerate_nodes(thunder_audio_compositor* self)
{
	thunder_sample source[THUNDER_ATOM_SAMPLE_COUNT];

	static const int LEFT_CHANNEL = 0;
	static const int RIGHT_CHANNEL = 1;
	static const int NUMBER_OF_OUTPUT_CHANNELS = 2;

	thunder_mix_sample* output = self->output;

	for (int i = 0; i < self->nodes_max_count; ++i) {
		thunder_audio_node* node = &self->nodes[i];

		if (node->is_playing) {
			if (node->channel_count == 2) {
				node->output(node->_self, source, THUNDER_ATOM_SAMPLE_COUNT / 2);
				add_to_output(output, 0, 1, source, node->volume);
			} else {
				node->output(node->_self, source, THUNDER_ATOM_SAMPLE_COUNT / 2);
				tyran_number normalized_pan = (node->pan + 1.0f) / 2.0f;
				add_to_output(output, LEFT_CHANNEL, NUMBER_OF_OUTPUT_CHANNELS, source, normalized_pan * node->volume);
				add_to_output(output, RIGHT_CHANNEL, NUMBER_OF_OUTPUT_CHANNELS, source, (1.0f - normalized_pan) * node->volume);
			}
		}
	}
}

static void compress_to_16_bit(thunder_audio_compositor* self, thunder_sample* output)
{
	thunder_mix_sample max_amplitude = maximum_amplitude(self->output, THUNDER_ATOM_SAMPLE_COUNT);

	adjust_mix_down_volume(self, max_amplitude);
	mix_down_using_volume(self->output, THUNDER_ATOM_SAMPLE_COUNT, self->mix_down_volume, output);
}

static void clear_buffer(thunder_mix_sample* output)
{
	tyran_mem_clear_type_n(output, THUNDER_ATOM_SAMPLE_COUNT);
}

void thunder_audio_compositor_update(thunder_audio_compositor* self)
{
	clear_buffer(self->output);
	enumerate_nodes(self);
	compress_to_16_bit(self, self->output_16_bit);
	thunder_audio_buffer_write(&self->buffer, self->output_16_bit, THUNDER_ATOM_SAMPLE_COUNT);
}

void thunder_audio_compositor_init(thunder_audio_compositor* self, struct tyran_memory* memory)
{
	self->output = TYRAN_MEMORY_CALLOC_TYPE_COUNT(memory, thunder_mix_sample, THUNDER_ATOM_SAMPLE_COUNT);
	self->output_16_bit = TYRAN_MEMORY_CALLOC_TYPE_COUNT(memory, thunder_sample, THUNDER_ATOM_SAMPLE_COUNT);
	self->nodes_max_count = 10;
	self->nodes = TYRAN_MEMORY_CALLOC_TYPE_COUNT(memory, thunder_audio_node, self->nodes_max_count);
	for (int i = 0; i < self->nodes_max_count; ++i) {
		self->nodes[i]._self = 0;
		self->nodes[i].output = 0;
	}

	thunder_audio_buffer_init(&self->buffer, memory, THUNDER_ATOM_SAMPLE_COUNT);

	self->mix_down_volume = 1;
}

void thunder_audio_compositor_free(thunder_audio_compositor* self)
{
	tyran_free(self->output);
	tyran_free(self->nodes);
	thunder_audio_buffer_free(&self->buffer);
}
