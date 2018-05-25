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
#ifndef thunder_sound_compositor_h
#define thunder_sound_compositor_h

#include <thunder/sound_buffer.h>
#include <thunder/sound_types.h>

struct tyran_memory;
struct thunder_audio_node;

typedef struct thunder_audio_compositor {
	struct thunder_audio_node* nodes;
	int nodes_max_count;
	int nodes_count;
	tyran_number mix_down_volume;
	thunder_audio_buffer buffer;
	thunder_mix_sample* output;
	thunder_sample* output_16_bit;
} thunder_audio_compositor;

void thunder_audio_compositor_init(thunder_audio_compositor* self, struct tyran_memory* memory);

void thunder_audio_compositor_update(thunder_audio_compositor* self);

void thunder_audio_compositor_free(thunder_audio_compositor* self);

#endif
