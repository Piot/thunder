/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <thunder/circular_buffer.h>

void thunderAudioCircularBufferInit(ThunderAudioCircularBuffer* self, size_t maxSize)
{
    self->buffer = tc_malloc_type_count(ThunderSampleOutputS16, maxSize);
    self->writeIndex = 0;
    self->readIndex = 0;
    self->maxSize = maxSize;
}

void thunderAudioCircularBufferDestroy(ThunderAudioCircularBuffer* self)
{
    tc_free(self->buffer);
}

size_t thunderAudioCircularBufferReadAvailable(const ThunderAudioCircularBuffer* self)
{
    return self->size;
}

void thunderAudioCircularBufferWrite(ThunderAudioCircularBuffer* self, const ThunderSample* data,
                                     size_t sampleCountInTarget)
{
    const ThunderSample* source = data;
    size_t sampleCount = sampleCountInTarget;

    if ((sampleCount % 2) != 0) {
        CLOG_ERROR("thunderAudioCircularBufferWrite: it was intended to be used for interleaved stereo")
    }

    size_t availableWriteCount = self->maxSize - self->size;
    if (sampleCount > availableWriteCount) {
        sampleCount = availableWriteCount;
    }

    if (sampleCount == 0) {
        return;
    }

    size_t firstAvailable = self->maxSize - self->writeIndex;
    size_t firstRun = sampleCount;
    if (firstRun > firstAvailable) {
        firstRun = firstAvailable;
    }
    tc_memcpy_type(ThunderSampleOutputS16, self->buffer + self->writeIndex, source, firstRun);

    sampleCount -= firstRun;
    source += firstRun;
    self->size += firstRun;
    self->writeIndex += firstRun;
    self->writeIndex %= self->maxSize;
    if (sampleCount == 0) {
        return;
    }

    tc_memcpy_type(ThunderSampleOutputS16, self->buffer + self->writeIndex, source, sampleCount);
    self->writeIndex += sampleCount;
    self->writeIndex %= self->maxSize;
    self->size += sampleCount;
}

void thunderAudioCircularBufferRead(ThunderAudioCircularBuffer* self, ThunderSample* output, size_t requiredCount)
{
    ThunderSample* target = output;

    size_t availableReadCount = self->size;
    size_t readCount = requiredCount;
    if (readCount > availableReadCount) {
        readCount = availableReadCount;
    }

    size_t availableFirstRun = self->maxSize - self->readIndex;
    size_t firstRun = readCount;
    if (readCount > availableFirstRun) {
        firstRun = availableFirstRun;
    }

    tc_memcpy_type(ThunderSampleOutputS16, target, self->buffer + self->readIndex, firstRun);
    target += firstRun;
    self->readIndex += firstRun;
    self->readIndex %= self->maxSize;
    self->size -= firstRun;
    readCount -= firstRun;

    tc_memcpy_type(ThunderSampleOutputS16, target, self->buffer + self->readIndex, readCount);
    self->readIndex += readCount;
    self->readIndex %= self->maxSize;
    self->size -= readCount;
    target += readCount;

    if (requiredCount > availableReadCount) {
        size_t rest = requiredCount - availableReadCount;
        tc_mem_clear_type_n(target, rest);
    }
}
