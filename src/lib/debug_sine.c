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
#include <thunder/sound_module.h>
#include <thunder/sound_types.h>
#include <tiny-libc/tiny_libc.h>
#include <basal/basal_math.h>
#include <stdio.h>

#include <math.h>

typedef struct debug_sine {
	uint64_t time;
	uint64_t timeFactor;
	uint64_t amplitude;
	uint64_t amplitudeFactor;
	float offset;
} debug_sine;

static void generate_sine(void* _self, thunder_sample* output, int sample_count)
{
	//printf("sine:%d\n", sample_count);
	debug_sine* self = _self;
	for (int i = 0; i < sample_count; ++i) {
		tyran_number angle = (self->time % 628318) / 100000.0f;
		tyran_number f = sinf(angle);
		tyran_number amplitude = self->amplitude / 1000000.0f;

		output[i] = (f  * 32767.0f * amplitude);
		self->time += self->timeFactor;
		self->amplitude += self->amplitudeFactor;
		if (self->amplitude > 1000000) {
			self->amplitude = 1000000;
		}
	}
}

thunder_audio_node make_sine_node(uint64_t offset, uint64_t factor, uint64_t amplitudeFactor)
{
	thunder_audio_node self;
	debug_sine* data = tc_malloc_type(debug_sine);

	self.output = generate_sine;
	self._self = data;
	self.is_playing = 1;
	self.channel_count = 1;
	self.pan = 0.0f;
	self.volume = 0.5f;

	data->offset = offset;
	data->timeFactor = factor;
	data->amplitude = 10;
	data->amplitudeFactor = amplitudeFactor;
	data->time = offset;


	return self;
}

void add_sine(thunder_sound_module* self, uint64_t offset, uint64_t factor, uint64_t amplitudeFactor)
{
    (void)offset;
    
	thunder_audio_node node = make_sine_node(offset, factor, amplitudeFactor);
	thunder_sound_module_add_node(self, node);
}

void thunder_sound_module_debug_sine_wave(thunder_sound_module* self, struct imprint_memory* memory)
{
    (void) memory;

	const uint64_t base = 4300;

	const uint64_t first = (1.4983 * base);
	const uint64_t second = (1.2599 * base);
	const uint64_t third = (2.0 * base);
	add_sine(self, 0, base, 30);
	add_sine(self, 1120, first, 20);
	add_sine(self, 530, second, 4);
	add_sine(self, 130, third, 6);
}
