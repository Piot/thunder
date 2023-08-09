/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef thunder_sound_buffer_h
#define thunder_sound_buffer_h

#include "sound_types.h"

struct ImprintAllocator;

typedef struct ThunderAudioBuffer {
    size_t atomSize;
    size_t bufferCount;
    ThunderSampleOutputS16** buffers;
    ThunderSampleOutputS16* readBuffer;
    size_t readBufferSamplesLeft;
    size_t writeIndex;
    size_t readIndex;
} ThunderAudioBuffer;

void thunderAudioBufferWrite(ThunderAudioBuffer* self, const ThunderSample* samples, size_t sample_count);

void thunderAudioBufferRead(ThunderAudioBuffer* self, ThunderSampleOutputS16* output, size_t sample_count);

void thunderAudioBufferInit(ThunderAudioBuffer* self, struct ImprintAllocator* memory, size_t atom_count,
                               size_t atom_size);

void thunderAudioBufferFree(ThunderAudioBuffer* self);

float thunder_audio_buffer_percentage_full(ThunderAudioBuffer* self);
size_t thunderAudioBufferAtomsFull(ThunderAudioBuffer* self);

#endif
