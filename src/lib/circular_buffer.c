#include <clog/clog.h>
#include <thunder/circular_buffer.h>
#include <tiny-libc/tiny_libc.h>

void thunderAudioCircularBufferInit(ThunderAudioCircularBuffer* self, size_t maxSize)
{
    self->buffer = tc_malloc_type_count(ThunderSampleOutputS16, maxSize);
    self->write_index = 0;
    self->read_index = 0;
    self->max_size = maxSize;
}

void thunderAudioCircularBufferDestroy(ThunderAudioCircularBuffer* self)
{
    tc_free(self->buffer);
}

int thunderAudioCircularBufferReadAvailable(const ThunderAudioCircularBuffer* self)
{
    return self->size;
}

void thunderAudioCircularBufferWrite(ThunderAudioCircularBuffer* self, const ThunderSample* data,
                                     size_t sampleCountInTarget)
{
    const ThunderSample* source = data;
    int sampleCount = sampleCountInTarget;

    if ((sampleCount % 2) != 0) {
        CLOG_ERROR("thunderAudioCircularBufferWrite: it was intended to be used for interleaved stereo");
    }

    int availableWriteCount = self->max_size - self->size;
    if (sampleCount > availableWriteCount) {
        sampleCount = availableWriteCount;
    }

    if (sampleCount == 0) {
        return;
    }

    int firstAvailable = self->max_size - self->write_index;
    int firstRun = sampleCount;
    if (firstRun > firstAvailable) {
        firstRun = firstAvailable;
    }
    tc_memcpy_type(ThunderSampleOutputS16, self->buffer + self->write_index, source, firstRun);

    sampleCount -= firstRun;
    source += firstRun;
    self->size += firstRun;
    self->write_index += firstRun;
    self->write_index %= self->max_size;
    if (sampleCount == 0) {
        return;
    }

    tc_memcpy_type(ThunderSampleOutputS16, self->buffer + self->write_index, source, sampleCount);
    self->write_index += sampleCount;
    self->write_index %= self->max_size;
    self->size += sampleCount;
}

void thunderAudioCircularBufferRead(ThunderAudioCircularBuffer* self, ThunderSample* output, size_t requiredCount)
{
    ThunderSample* target = output;

    int availableReadCount = self->size;
    int readCount = requiredCount;
    if (readCount > availableReadCount) {
        readCount = availableReadCount;
    }

    int availableFirstRun = self->max_size - self->read_index;
    int firstRun = readCount;
    if (readCount > availableFirstRun) {
        firstRun = availableFirstRun;
    }

    tc_memcpy_type(ThunderSampleOutputS16, target, self->buffer + self->read_index, firstRun);
    target += firstRun;
    self->read_index += firstRun;
    self->read_index %= self->max_size;
    self->size -= firstRun;
    readCount -= firstRun;

    tc_memcpy_type(ThunderSampleOutputS16, target, self->buffer + self->read_index, readCount);
    self->read_index += readCount;
    self->read_index %= self->max_size;
    self->size -= readCount;
    target += readCount;

    if (requiredCount > availableReadCount) {
        int rest = requiredCount - availableReadCount;
        tc_mem_clear_type_n(target, rest);
    }
}
