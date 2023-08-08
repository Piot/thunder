/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <imprint/allocator.h>
#include <thunder/sound_buffer.h>

void thunder_audio_buffer_write(thunder_audio_buffer* self, const ThunderSample* samples, size_t sample_count)
{
    CLOG_ASSERT(self->atom_size == sample_count, "Wrong store size:%d", sample_count);

    if (self->write_index == self->read_index) {
        // return;
    }

    size_t index = self->write_index;

    ThunderSampleOutputS16* buffer = self->buffers[index];

    tc_memcpy_octets(buffer, samples, sample_count * sizeof(ThunderSample));

    if (!self->read_buffer) {
        self->read_buffer = self->buffers[0];
        self->read_buffer_samples_left = self->atom_size;
    }

    self->write_index++;
    self->write_index %= self->buffer_count;
}

// NOTE: IMPORTANT: You can not allocate mewmory or use mutex when reading!
void thunder_audio_buffer_read(thunder_audio_buffer* self, ThunderSampleOutputS16* output, size_t sample_count)
{
    size_t samples_to_read = sample_count;

    if (samples_to_read > self->read_buffer_samples_left) {
        samples_to_read = self->read_buffer_samples_left;
    }
    tc_memcpy_type(ThunderSampleOutputS16, output, self->read_buffer, samples_to_read);
    sample_count -= samples_to_read;
    self->read_buffer_samples_left -= samples_to_read;

    if (self->read_buffer_samples_left == 0) {
        if (self->write_index == self->read_index) {
            ThunderSampleOutputS16* zero = output + samples_to_read;
            tc_mem_clear_type_n(zero, sample_count);
            return;
        }
        self->read_index++;
        self->read_index %= self->buffer_count;
        self->read_buffer = self->buffers[self->read_index];
        self->read_buffer_samples_left = self->atom_size;
    } else {
        self->read_buffer += samples_to_read;
    }

    if (sample_count > 0) {
        thunder_audio_buffer_read(self, output + samples_to_read, sample_count);
    }
}

void thunder_audio_buffer_init(thunder_audio_buffer* self, struct ImprintAllocator* memory, size_t bufferCount,
                               size_t atom_size)
{
    self->buffer_count = bufferCount;
    self->atom_size = atom_size;
    self->read_index = (size_t) -1;
    self->write_index = 0;
    self->buffers = IMPRINT_CALLOC_TYPE_COUNT(memory, ThunderSampleOutputS16*, self->buffer_count);
    for (size_t i = 0; i < self->buffer_count; ++i) {
        self->buffers[i] = IMPRINT_CALLOC_TYPE_COUNT(memory, ThunderSampleOutputS16, atom_size);
    }
    self->read_buffer = 0;
    self->read_buffer_samples_left = 0;
}

float thunder_audio_buffer_percentage_full(thunder_audio_buffer* self)
{
    size_t diff = thunder_audio_buffer_atoms_full(self);
    return (float) diff / (float) self->buffer_count;
}

size_t thunder_audio_buffer_atoms_full(thunder_audio_buffer* self)
{
    size_t diff;

    if (self->write_index == self->read_index) {
        return 0;
    }

    if (self->write_index > self->read_index) {
        diff = self->write_index - self->read_index;
    } else {
        diff = self->buffer_count - self->read_index + self->write_index;
    }

    return diff;
}

void thunder_audio_buffer_free(thunder_audio_buffer* self)
{
    for (size_t i = 0; i < self->buffer_count; ++i) {
        tc_free(self->buffers[i]);
    }
    tc_free(self->buffers);
    self->buffer_count = 0;
}
