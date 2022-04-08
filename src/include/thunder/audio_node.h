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
#ifndef thunder_audio_node_h
#define thunder_audio_node_h

#include <thunder/sound_types.h>
#include <stdbool.h>

typedef void (*ThunderAudioNodeOutputFunc)(void* self, ThunderSample* sample, int sample_count);

typedef struct ThunderAudioNode {
    ThunderAudioNodeOutputFunc output;
    void* _self;
    bool is_playing;
    int channel_count;
    int pan;
    float volume;
} ThunderAudioNode;

void thunder_audio_node_init(ThunderAudioNode* self, ThunderAudioNodeOutputFunc func, void* user_data);

#endif
