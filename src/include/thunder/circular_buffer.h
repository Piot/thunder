/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef thunder_sound_circular_buffer_h
#define thunder_sound_circular_buffer_h

#include <stddef.h>
#include <thunder-audio-buffer/sound_types.h>

typedef struct ThunderAudioCircularBuffer {
    size_t maxSize;
    ThunderSampleOutputS16* buffer;
    size_t writeIndex;
    size_t readIndex;
    size_t size;
} ThunderAudioCircularBuffer;

void thunderAudioCircularBufferInit(ThunderAudioCircularBuffer* self, size_t maxSize);
void thunderAudioCircularBufferDestroy(ThunderAudioCircularBuffer* self);
size_t thunderAudioCircularBufferReadAvailable(const ThunderAudioCircularBuffer* self);
void thunderAudioCircularBufferWrite(ThunderAudioCircularBuffer* self, const ThunderSample* data,
                                     size_t sampleCountInTarget);
void thunderAudioCircularBufferRead(ThunderAudioCircularBuffer* self, ThunderSample* output, size_t requiredCount);

#endif
