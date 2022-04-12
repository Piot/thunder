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

struct ImprintMemory;
struct ThunderAudioNode;

typedef struct ThunderAudioCompositor {
    struct ThunderAudioNode* nodes;
    int nodes_max_count;
    int nodes_count;
    float mix_down_volume;
    thunder_audio_buffer buffer;
    ThunderMixSample* output;
    ThunderSample* output_16_bit;
} ThunderAudioCompositor;

void thunder_audio_compositor_init(ThunderAudioCompositor* self, struct ImprintAllocator* memory);

void thunder_audio_compositor_update(ThunderAudioCompositor* self);

void thunder_audio_compositor_reload(ThunderAudioCompositor* self);

void thunder_audio_compositor_free(ThunderAudioCompositor* self);

#endif
