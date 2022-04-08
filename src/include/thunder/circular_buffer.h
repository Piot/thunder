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
#ifndef thunder_sound_circular_buffer_h
#define thunder_sound_circular_buffer_h

#include <thunder/sound_types.h>
#include <stddef.h>

typedef struct ThunderAudioCircularBuffer {
    int max_size;
    ThunderSampleOutputS16* buffer;
    int read_buffer_samples_left;
    int write_index;
    int read_index;
    int size;
} ThunderAudioCircularBuffer;

void thunderAudioCircularBufferInit(ThunderAudioCircularBuffer* self, size_t maxSize);
void thunderAudioCircularBufferDestroy(ThunderAudioCircularBuffer* self);
int thunderAudioCircularBufferReadAvailable(const ThunderAudioCircularBuffer* self);
void thunderAudioCircularBufferWrite(ThunderAudioCircularBuffer* self, const ThunderSample* data,
                                     size_t sampleCountInTarget);
void thunderAudioCircularBufferRead(ThunderAudioCircularBuffer* self, ThunderSample* output, size_t requiredCount);

#endif
