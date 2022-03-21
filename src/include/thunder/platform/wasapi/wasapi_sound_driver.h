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

#ifndef thunder_wasapi_sound_driver_impl_h
#define thunder_wasapi_sound_driver_impl_h

struct thunder_audio_buffer;
struct IMMDevice;
struct IAudioClient;
struct IAudioRenderClient;

#include <stdbool.h>

typedef struct thunder_wasapi_sound_driver {
    struct thunder_audio_buffer* buffer;
    struct IMMDevice* device;
    struct IAudioClient* audioClient;
    struct  IAudioRenderClient* renderClient;
    unsigned int maxBufferFrameCount;
    void* thread;
} thunder_wasapi_sound_driver;

int thunder_wasapi_sound_driver_init(thunder_wasapi_sound_driver* self, struct thunder_audio_buffer* buffer,
                                          bool use_floats);
void thunder_wasapi_sound_driver_free(thunder_wasapi_sound_driver* self);

#endif
