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

#ifndef ThunderXaudio2SoundDriver_impl_h
#define ThunderXaudio2SoundDriver_impl_h

struct thunder_audio_buffer;
struct IXAudio2;
struct IXAudio2SourceVoice;

#include <stdbool.h>
#include <thunder/sound_types.h>

typedef struct ThunderXaudio2SoundDriver {
    struct thunder_audio_buffer* buffer;
    struct IXAudio2* xaudio2;
    struct IXAudio2MasteringVoice* masteringVoice;
    struct IXAudio2SourceVoice* sourceVoice;
    ThunderSampleOutputS16* buffers[2];
    int bufferAtomSizeInOctets;
    int bufferAtomSizeInBlocks;
    int activeBuffer;
} ThunderXaudio2SoundDriver;

int thunderXaudio2SoundDriverInit(ThunderXaudio2SoundDriver* self, struct thunder_audio_buffer* buffer,
                                      bool use_floats);
void thunderXaudio2SoundDriverFree(ThunderXaudio2SoundDriver* self);

#endif
