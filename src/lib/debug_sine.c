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

typedef struct debug_sine {
	int time;
	int max_count;
	int offset;
} debug_sine;

static void generate_sine(void* _self, thunder_sample* output, int sample_count)
{
	debug_sine* self = _self;
	for (int i = 0; i < sample_count; ++i) {
		tyran_number f = sinf(self->offset + (6.28f * (tyran_number) self->time) / (tyran_number) self->max_count);
		output[i] = f * 32767;
		self->time++;
	}
}

thunder_audio_node make_sine_node(int max_count)
{
	thunder_audio_node self;
	debug_sine* data = tc_malloc_type(debug_sine);

	self.output = generate_sine;
	self._self = data;
	self.is_playing = 1;
	self.channel_count = 1;
	self.pan = 0.0f;
	data->offset = max_count;
	data->max_count = max_count;

	return self;
}

void add_sine(thunder_sound_module* self, int max_count, int offset)
{
    (void)offset;
    
	thunder_audio_node node = make_sine_node(max_count);
	thunder_sound_module_add_node(self, node);
}

void thunder_sound_module_debug_sine_wave(thunder_sound_module* self, struct imprint_memory* memory)
{
    (void) memory;
    
	add_sine(self, 128, 1);
	add_sine(self, 64, 2);
	add_sine(self, 256, 0);
}
